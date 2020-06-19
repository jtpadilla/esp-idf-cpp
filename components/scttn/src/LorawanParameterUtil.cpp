
#include "esp_log.h"

#include "LorawanParameterUtil.h"

namespace scttn
{
    
    LorawanParameter LorawanParameterUtil::convert(const char devEuiParam[], const char appEuiParam[], const char appKeyParam[]) {

    }

    LorawanParameter LorawanParameterUtil::convert(const char appEuiParam[], const char appKeyParam[]) {

    }


    bool LorawanParameterUtil::decodeKeys(const char *dev_eui, const char *app_eui, const char *app_key)
    {
        return decode(true, dev_eui, app_eui, app_key);
    }

    bool LorawanParameterUtil::fromMAC(const char *app_eui, const char *app_key)
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

    bool LorawanParameterUtil::decode(bool incl_dev_eui, const char *dev_eui, const char *app_eui, const char *app_key)
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

        if (incl_dev_eui) {
            memcpy(global_dev_eui, buf_dev_eui, sizeof(global_dev_eui));
        }

        memcpy(global_app_eui, buf_app_eui, sizeof(global_app_eui));
        memcpy(global_app_key, buf_app_key, sizeof(global_app_key));

            have_keys = !isAllZeros(global_dev_eui, sizeof(global_dev_eui))
                && !isAllZeros(global_app_eui, sizeof(global_app_eui))
                && !isAllZeros(global_app_key, sizeof(global_app_key));

        return true;
    }

    bool LorawanParameterUtil::hexStrToBin(const char *hex, uint8_t *buf, int len)
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

    int LorawanParameterUtil::hexTupleToByte(const char *hex)
    {
        int nibble1 = hexDigitToVal(hex[0]);
        if (nibble1 < 0)
            return -1;
        int nibble2 = hexDigitToVal(hex[1]);
        if (nibble2 < 0)
            return -1;
        return (nibble1 << 4) | nibble2;
    }

    int LorawanParameterUtil::hexDigitToVal(char ch)
    {
        if (ch >= '0' && ch <= '9')
            return ch - '0';
        if (ch >= 'A' && ch <= 'F')
            return ch + 10 - 'A';
        if (ch >= 'a' && ch <= 'f')
            return ch + 10 - 'a';
        return -1;
    }

    void LorawanParameterUtil::binToHexStr(const uint8_t* buf, int len, char* hex)
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

    char LorawanParameterUtil::valToHexDigit(int val)
    {
        return "0123456789ABCDEF"[val];
    }

    void LorawanParameterUtil::swapBytes(uint8_t* buf, int len)
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

    bool LorawanParameterUtil::isAllZeros(const uint8_t* buf, int len)
    {
        for (int i = 0; i < len; i++)
            if (buf[i] != 0)
                return false;
        return true;
    }

}