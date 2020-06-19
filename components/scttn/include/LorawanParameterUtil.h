#pragma once

#include <string>
#include "LorawanParameter.h"

namespace scttn
{

    class LorawanParameterUtil
    {

        public:
            static scttn::LorawanParameter convert(std:string appEui, std:string appKey, sdt::string devEui);
            static scttn::LorawanParameter convert(std:string appEui, std:string appKey);

        private:
            void decode(LorawanParameter& lorawanParameter, std:string& appEui, std:string& appKey, sdt::string& devEui);
            void putMAC(LorawanParameter& lorawanParameter);

            static bool hexStrToBin(const char *hex, uint8_t *buf, int len);
            static int hexTupleToByte(const char *hex);
            static int hexDigitToVal(char ch);
            static void binToHexStr(const uint8_t* buf, int len, char* hex);
            static char valToHexDigit(int val);
            static void swapBytes(uint8_t* buf, int len);
            static bool isAllZeros(const uint8_t* buf, int len);

    };

}
