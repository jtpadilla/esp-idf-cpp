#pragma once

#include <string>
#include "LorawanParameter.h"

namespace sc::lorawan
{

    class LorawanParameterUtil
    {

        public:
            static sc:Lorawan::LorawanParameter convert(std:string appEui, std:string appKey, sdt::string devEui);
            static sc:Lorawan::LorawanParameter convert(std:string appEui, std:string appKey);

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
