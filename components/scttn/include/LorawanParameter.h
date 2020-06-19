#pragma once

#include <cstdint>

namespace scttn
{
    
    struct LorawanParameter {
        uint8_t devEui[8];
        uint8_t appEui[8];
        uint8_t appKey[16];
    };

}