#pragma once

#include <string>

namespace scttn
{

    class LorawanParameterUtil
    {

        public:
            static scttn::LorawanParameter convert(const char devEuiParam[], const char appEuiParam[], const char appKeyParam[]);
            static scttn::LorawanParameter convert(const char appEuiParam[], const char appKeyParam[]);

        private:
            static bool hexStrToBin(const char *hex, uint8_t *buf, int len);
            static int hexTupleToByte(const char *hex);
            static int hexDigitToVal(char ch);
            static void binToHexStr(const uint8_t* buf, int len, char* hex);
            static char valToHexDigit(int val);
            static void swapBytes(uint8_t* buf, int len);
            static bool isAllZeros(const uint8_t* buf, int len);

    };

}
