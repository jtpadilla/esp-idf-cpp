#pragma once

class TtnProvisioning
{
    public:
        TtnProvisioning(const char *dev_eui, const char *app_eui, const char *app_key);
        TtnProvisioning(const char *app_eui, const char *app_key);
        TtnProvisioning(uint8_t devEuiParam[8], uint8_t appEuiParam[8], uint8_t appKeyParam[16]);

    private:
        bool decode(bool incl_dev_eui, const char *dev_eui, const char *app_eui, const char *app_key);

        uint8_t devEui[8];
        uint8_t appEui[8];
        uint8_t appKey[16];

        static bool hexStrToBin(const char *hex, uint8_t *buf, int len);
        static int hexTupleToByte(const char *hex);
        static int hexDigitToVal(char ch);
        static void binToHexStr(const uint8_t* buf, int len, char* hex);
        static char valToHexDigit(int val);
        static void swapBytes(uint8_t* buf, int len);
        static bool isAllZeros(const uint8_t* buf, int len);

};

