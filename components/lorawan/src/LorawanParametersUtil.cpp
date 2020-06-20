
#include "esp_log.h"
#include "esp_system.h"

#include <cstring>

#include "LorawanParametersUtil.h"

namespace genielink::lorawan
{
    
    genielink::lorawan::LorawanParameters LorawanParametersUtil::convert(std::string appEui, std::string appKey, std::string devEui) {
        LorawanParameters lorawanParameters{};
        decodeAppEud(lorawanParameters, appEui);
        decodeAppKey(lorawanParameters, appKey);
        decodeDevEui(lorawanParameters, devEui);
        return lorawanParameters;
    }

    genielink::lorawan::LorawanParameters LorawanParametersUtil::convert(std::string appEui, std::string appKey) {
        LorawanParameters lorawanParameters{};
        decodeAppEud(lorawanParameters, appEui);
        decodeAppKey(lorawanParameters, appKey);
        loadDevEuiFromMAC(lorawanParameters);
        return lorawanParameters;
    }

    void LorawanParametersUtil::decodeAppEud(LorawanParameters& lorawanParameters, std::string& appEui) {
        uint8_t buf_app_eui[8];
        if (appEui.length() != 16 || !hexStrToBin(appEui.c_str(), buf_app_eui, 8))
        {
            throw genielink::lorawan::LorawanParametersException{"Invalid application EUI: " + appEui};
        }
        swapBytes(buf_app_eui, 8);
        std::memcpy(&lorawanParameters.appEui, buf_app_eui, sizeof(lorawanParameters.appEui));
    }

    void LorawanParametersUtil::decodeAppKey(LorawanParameters& lorawanParameters, std::string& appKey) {
        uint8_t buf_app_key[16];
        if (appKey.length() != 32 || !hexStrToBin(appKey.c_str(), buf_app_key, 16))
        {
            throw genielink::lorawan::LorawanParametersException{"Invalid application key: " + appKey};
        }
        std::memcpy(&lorawanParameters.appKey, buf_app_key, sizeof(lorawanParameters.appKey));
    }

    void LorawanParametersUtil::decodeDevEui(LorawanParameters& lorawanParameters, std::string& devEui) {
        uint8_t buf_dev_eui[8];
        if (devEui.length() != 16 || !hexStrToBin(devEui.c_str(), buf_dev_eui, 8))
        {
            throw genielink::lorawan::LorawanParametersException{"Invalid device EUI: " + devEui};
        }
        swapBytes(buf_dev_eui, 8);
        std::memcpy(&lorawanParameters.devEui, buf_dev_eui, sizeof(lorawanParameters.devEui));
    }
    
    void LorawanParametersUtil::loadDevEuiFromMAC(LorawanParameters& lorawanParameters) {

        // Obtiene la MAC del hardware
        uint8_t mac[6];
        esp_err_t err = esp_efuse_mac_get_default(mac);
        ESP_ERROR_CHECK(err);

        // Se coloca en su sitio
        uint8_t buf_dev_eui[8];
        buf_dev_eui[7] = mac[0];
        buf_dev_eui[6] = mac[1];
        buf_dev_eui[5] = mac[2];
        buf_dev_eui[4] = 0xff;
        buf_dev_eui[3] = 0xfe;
        buf_dev_eui[2] = mac[3];
        buf_dev_eui[1] = mac[4];
        buf_dev_eui[0] = mac[5];

        std::memcpy(&lorawanParameters.devEui, buf_dev_eui, sizeof(lorawanParameters.devEui));
        
    }

    bool LorawanParametersUtil::hexStrToBin(const char *hex, uint8_t *buf, int len)
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

    int LorawanParametersUtil::hexTupleToByte(const char *hex)
    {
        int nibble1 = hexDigitToVal(hex[0]);
        if (nibble1 < 0)
            return -1;
        int nibble2 = hexDigitToVal(hex[1]);
        if (nibble2 < 0)
            return -1;
        return (nibble1 << 4) | nibble2;
    }

    int LorawanParametersUtil::hexDigitToVal(char ch)
    {
        if (ch >= '0' && ch <= '9')
            return ch - '0';
        if (ch >= 'A' && ch <= 'F')
            return ch + 10 - 'A';
        if (ch >= 'a' && ch <= 'f')
            return ch + 10 - 'a';
        return -1;
    }

    void LorawanParametersUtil::binToHexStr(const uint8_t* buf, int len, char* hex)
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

    char LorawanParametersUtil::valToHexDigit(int val)
    {
        return "0123456789ABCDEF"[val];
    }

    void LorawanParametersUtil::swapBytes(uint8_t* buf, int len)
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

    bool LorawanParametersUtil::isAllZeros(const uint8_t* buf, int len)
    {
        for (int i = 0; i < len; i++)
            if (buf[i] != 0)
                return false;
        return true;
    }

}