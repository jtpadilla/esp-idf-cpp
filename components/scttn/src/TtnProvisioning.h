#pragma once

#include "lmic/oslmic.h"
#include "nvs_flash.h"


class TtnProvisioning
{
    public:
        TtnProvisioning();

        bool haveKeys();
        bool decodeKeys(const char *dev_eui, const char *app_eui, const char *app_key);
        bool fromMAC(const char *app_eui, const char *app_key);
        bool saveKeys();
        bool restoreKeys(bool silent);

    private:
        bool decode(bool incl_dev_eui, const char *dev_eui, const char *app_eui, const char *app_key);
        bool readNvsValue(nvs_handle handle, const char* key, uint8_t* data, size_t expected_length, bool silent);
        bool writeNvsValue(nvs_handle handle, const char* key, const uint8_t* data, size_t len);

        static bool hexStrToBin(const char *hex, uint8_t *buf, int len);
        static int hexTupleToByte(const char *hex);
        static int hexDigitToVal(char ch);
        static void binToHexStr(const uint8_t* buf, int len, char* hex);
        static char valToHexDigit(int val);
        static void swapBytes(uint8_t* buf, int len);
        static bool isAllZeros(const uint8_t* buf, int len);

    private:
        bool have_keys = false;

};

