#pragma once

#include "TheThingsNetwork.h"

namespace speedycontrol::ttn
{
    class ITtnTaskFactory
    {
        public:
            virtual void createAndLaunch(TheThingsNetwork& ttn) = 0;
    };

}
