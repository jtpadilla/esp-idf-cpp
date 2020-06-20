#pragma once

#include <string>
#include "LorawanParameters.h"
#include "LorawanParametersException.h"

namespace genielink::lorawan
{

    class LorawanParametersUtil
    {

        public:
            static genielink::lorawan::LorawanParameters convert(std::string appEui, std::string appKey, std::string devEui);
            static genielink::lorawan::LorawanParameters convert(std::string appEui, std::string appKey);

        private:
            static void decode(LorawanParameters& lorawanParameters, std::string& appEui, std::string& appKey, std::string& devEui);
            static void decodeAppEud(LorawanParameters& lorawanParameters, std::string& appEui);
            static void decodeAppKey(LorawanParameters& lorawanParameters, std::string& appKey);
            static void decodeDevEui(LorawanParameters& lorawanParameters, std::string& devEui);
            static void loadDevEuiFromMAC(LorawanParameters& lorawanParameters);

            static bool hexStrToBin(const char *hex, uint8_t *buf, int len);
            static int hexTupleToByte(const char *hex);
            static int hexDigitToVal(char ch);
            static void binToHexStr(const uint8_t* buf, int len, char* hex);
            static char valToHexDigit(int val);
            static void swapBytes(uint8_t* buf, int len);
            static bool isAllZeros(const uint8_t* buf, int len);

    };

}
