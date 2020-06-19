#pragma once

#include "LorawanDriverPins.h"

namespace sc::lorawan
{

    class LorawanDriverPinsTtgoTBeam: private LorawanDriverPins
    {
        public:

            LorawanDriverPinsTtgoTBeam():
                LorawanDriverPins(HSPI_HOST, 1, 5, 27, 19, 18, NOT_CONNECTED, 23, 26, 33)
            {
            }

    }

}
