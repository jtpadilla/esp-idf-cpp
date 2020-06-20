#pragma once

#include <cstdint>

namespace genielink::lorawan
{
    
    struct LorawanParameters {
        uint8_t devEui[8];
        uint8_t appEui[8];
        uint8_t appKey[16];
    };

}