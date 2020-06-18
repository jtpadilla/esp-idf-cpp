/*******************************************************************************
 * 
 * ttn-esp32 - The Things Network device library for ESP-IDF / SX127x
 * 
 * Copyright (c) 2018-2019 Manuel Bleichenbacher
 * 
 * Licensed under MIT License
 * https://opensource.org/licenses/MIT
 *
 * Task listening on a UART port for provisioning commands.
 *******************************************************************************/

#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "TtnProvisioning.h"
#include "lmic/lmic.h"
#include "hal/hal_esp32.h"

static const char* const TAG = "ttn_prov";
static const char* const NVS_FLASH_PARTITION = "ttn";
static const char* const NVS_FLASH_KEY_DEV_EUI = "devEui";
static const char* const NVS_FLASH_KEY_APP_EUI = "appEui";
static const char* const NVS_FLASH_KEY_APP_KEY = "appKey";

static uint8_t global_dev_eui[8];
static uint8_t global_app_eui[8];
static uint8_t global_app_key[16];


// --- LMIC callbacks

// This EUI must be in little-endian format, so least-significant-byte first.
// When copying an EUI from ttnctl output, this means to reverse the bytes.
// For TTN issued EUIs the last bytes should be 0xD5, 0xB3, 0x70.
// The order is swapped in provisioning_decode_keys().
void os_getArtEui (u1_t* buf)
{
    memcpy(buf, global_app_eui, 8);
}

// This should also be in little endian format, see above.
void os_getDevEui (u1_t* buf)
{
    memcpy(buf, global_dev_eui, 8);
}

// This key should be in big endian format (or, since it is not really a number
// but a block of memory, endianness does not really apply). In practice, a key
// taken from ttnctl can be copied as-is.
void os_getDevKey (u1_t* buf)
{
    memcpy(buf, global_app_key, 16);
}

// --- Constructor

TtnProvisioning::TtnProvisioning()
    : have_keys(false)
{
}


bool TtnProvisioning::haveKeys()
{
    return have_keys;
}

bool TtnProvisioning::decodeKeys(const char *dev_eui, const char *app_eui, const char *app_key)
{
    return decode(true, dev_eui, app_eui, app_key);
}

bool TtnProvisioning::fromMAC(const char *app_eui, const char *app_key)
{
    uint8_t mac[6];
    esp_err_t err = esp_efuse_mac_get_default(mac);
    ESP_ERROR_CHECK(err);
    
    global_dev_eui[7] = mac[0];
    global_dev_eui[6] = mac[1];
    global_dev_eui[5] = mac[2];
    global_dev_eui[4] = 0xff;
    global_dev_eui[3] = 0xfe;
    global_dev_eui[2] = mac[3];
    global_dev_eui[1] = mac[4];
    global_dev_eui[0] = mac[5];

    return decode(false, nullptr, app_eui, app_key);
}

bool TtnProvisioning::decode(bool incl_dev_eui, const char *dev_eui, const char *app_eui, const char *app_key)
{
    uint8_t buf_dev_eui[8];
    uint8_t buf_app_eui[8];
    uint8_t buf_app_key[16];

    if (incl_dev_eui && (strlen(dev_eui) != 16 || !hexStrToBin(dev_eui, buf_dev_eui, 8)))
    {
        ESP_LOGW(TAG, "Invalid device EUI: %s", dev_eui);
        return false;
    }

    if (incl_dev_eui)
        swapBytes(buf_dev_eui, 8);

    if (strlen(app_eui) != 16 || !hexStrToBin(app_eui, buf_app_eui, 8))
    {
        ESP_LOGW(TAG, "Invalid application EUI: %s", app_eui);
        return false;
    }

    swapBytes(buf_app_eui, 8);

    if (strlen(app_key) != 32 || !hexStrToBin(app_key, buf_app_key, 16))
    {
        ESP_LOGW(TAG, "Invalid application key: %s", app_key);
        return false;
    }

    if (incl_dev_eui)
        memcpy(global_dev_eui, buf_dev_eui, sizeof(global_dev_eui));
    memcpy(global_app_eui, buf_app_eui, sizeof(global_app_eui));
    memcpy(global_app_key, buf_app_key, sizeof(global_app_key));

    have_keys = !isAllZeros(global_dev_eui, sizeof(global_dev_eui))
        && !isAllZeros(global_app_eui, sizeof(global_app_eui))
        && !isAllZeros(global_app_key, sizeof(global_app_key));

    if (!saveKeys())
        return false;
    
    return true;
}


// --- Non-volatile storage

bool TtnProvisioning::saveKeys()
{
    bool result = false;

    nvs_handle handle = 0;
    esp_err_t res = nvs_open(NVS_FLASH_PARTITION, NVS_READWRITE, &handle);
    if (res == ESP_ERR_NVS_NOT_INITIALIZED)
    {
        ESP_LOGW(TAG, "NVS storage is not initialized. Call 'nvs_flash_init()' first.");
        goto done;
    }
    ESP_ERROR_CHECK(res);
    if (res != ESP_OK)
        goto done;

    if (!writeNvsValue(handle, NVS_FLASH_KEY_DEV_EUI, global_dev_eui, sizeof(global_dev_eui)))
        goto done;
        
    if (!writeNvsValue(handle, NVS_FLASH_KEY_APP_EUI, global_app_eui, sizeof(global_app_eui)))
        goto done;
        
    if (!writeNvsValue(handle, NVS_FLASH_KEY_APP_KEY, global_app_key, sizeof(global_app_key)))
        goto done;

    res = nvs_commit(handle);
    ESP_ERROR_CHECK(res);
    
    result = true;
    ESP_LOGI(TAG, "Dev and app EUI and app key saved in NVS storage");

done:
    nvs_close(handle);
    return result;
}

bool TtnProvisioning::restoreKeys(bool silent)
{
    uint8_t buf_dev_eui[8];
    uint8_t buf_app_eui[8];
    uint8_t buf_app_key[16];
    
    nvs_handle handle = 0;
    esp_err_t res = nvs_open(NVS_FLASH_PARTITION, NVS_READONLY, &handle);
    if (res == ESP_ERR_NVS_NOT_FOUND)
        return false; // partition does not exist yet
    if (res == ESP_ERR_NVS_NOT_INITIALIZED)
    {
        ESP_LOGW(TAG, "NVS storage is not initialized. Call 'nvs_flash_init()' first.");
        goto done;
    }
    ESP_ERROR_CHECK(res);
    if (res != ESP_OK)
        goto done;

    if (!readNvsValue(handle, NVS_FLASH_KEY_DEV_EUI, buf_dev_eui, sizeof(global_dev_eui), silent))
        goto done;

    if (!readNvsValue(handle, NVS_FLASH_KEY_APP_EUI, buf_app_eui, sizeof(global_app_eui), silent))
        goto done;

    if (!readNvsValue(handle, NVS_FLASH_KEY_APP_KEY, buf_app_key, sizeof(global_app_key), silent))
        goto done;

    memcpy(global_dev_eui, buf_dev_eui, sizeof(global_dev_eui));
    memcpy(global_app_eui, buf_app_eui, sizeof(global_app_eui));
    memcpy(global_app_key, buf_app_key, sizeof(global_app_key));

    have_keys = !isAllZeros(global_dev_eui, sizeof(global_dev_eui))
        && !isAllZeros(global_app_eui, sizeof(global_app_eui))
        && !isAllZeros(global_app_key, sizeof(global_app_key));

    if (have_keys)
    {
       ESP_LOGI(TAG, "Dev and app EUI and app key have been restored from NVS storage");
    }
    else
    {
        ESP_LOGW(TAG, "Dev and app EUI and app key are invalid (zeroes only)");
    }

done:
    nvs_close(handle);
    return true;
}

bool TtnProvisioning::readNvsValue(nvs_handle handle, const char* key, uint8_t* data, size_t expected_length, bool silent)
{
    size_t size = expected_length;
    esp_err_t res = nvs_get_blob(handle, key, data, &size);
    if (res == ESP_OK && size == expected_length)
        return true;

    if (res == ESP_OK && size != expected_length)
    {
        if (!silent)
            ESP_LOGW(TAG, "Invalid size of NVS data for %s", key);
        return false;
    }

    if (res == ESP_ERR_NVS_NOT_FOUND)
    {
        if (!silent)
            ESP_LOGW(TAG, "No NVS data found for %s", key);
        return false;
    }
    
    ESP_ERROR_CHECK(res);
    return false;
}

bool TtnProvisioning::writeNvsValue(nvs_handle handle, const char* key, const uint8_t* data, size_t len)
{
    uint8_t buf[16];
    if (readNvsValue(handle, key, buf, len, true) && memcmp(buf, data, len) == 0)
        return true; // unchanged
    
    esp_err_t res = nvs_set_blob(handle, key, data, len);
    ESP_ERROR_CHECK(res);

    return res == ESP_OK;
}


// --- Helper functions ---

bool TtnProvisioning::hexStrToBin(const char *hex, uint8_t *buf, int len)
{
    const char* ptr = hex;
    for (int i = 0; i < len; i++)
    {
        int val = hexTupleToByte(ptr);
        if (val < 0)
            return false;
        buf[i] = val;
        ptr += 2;
    }
    return true;
}

int TtnProvisioning::hexTupleToByte(const char *hex)
{
    int nibble1 = hexDigitToVal(hex[0]);
    if (nibble1 < 0)
        return -1;
    int nibble2 = hexDigitToVal(hex[1]);
    if (nibble2 < 0)
        return -1;
    return (nibble1 << 4) | nibble2;
}

int TtnProvisioning::hexDigitToVal(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch + 10 - 'A';
    if (ch >= 'a' && ch <= 'f')
        return ch + 10 - 'a';
    return -1;
}

void TtnProvisioning::binToHexStr(const uint8_t* buf, int len, char* hex)
{
    for (int i = 0; i < len; i++)
    {
        uint8_t b = buf[i];
        *hex = valToHexDigit((b & 0xf0) >> 4);
        hex++;
        *hex = valToHexDigit(b & 0x0f);
        hex++;
    }
}

char TtnProvisioning::valToHexDigit(int val)
{
    return "0123456789ABCDEF"[val];
}

void TtnProvisioning::swapBytes(uint8_t* buf, int len)
{
    uint8_t* p1 = buf;
    uint8_t* p2 = buf + len - 1;
    while (p1 < p2)
    {
        uint8_t t = *p1;
        *p1 = *p2;
        *p2 = t;
        p1++;
        p2--;
    }
}

bool TtnProvisioning::isAllZeros(const uint8_t* buf, int len)
{
    for (int i = 0; i < len; i++)
        if (buf[i] != 0)
            return false;
    return true;
}