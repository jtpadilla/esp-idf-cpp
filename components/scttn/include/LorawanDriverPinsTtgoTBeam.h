#pragma once

#include "LorawanDriverPins.h"

namespace genielink::lorawan
{

    class LorawanDriverPinsTtgoTBeam: public LorawanDriverPins
    {
        public:
            LorawanDriverPinsTtgoTBeam():
                LorawanDriverPins(HSPI_HOST, 1, 5, 27, 19, 18, NOT_CONNECTED, 23, 26, 33)
            {
            }

    };

}
