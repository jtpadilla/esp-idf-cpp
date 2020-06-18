#pragma once

#include "TheThingsNetwork.h"

namespace scttn
{
    class ITtnTaskFactory
    {
        public:
            virtual void createAndLaunch(TheThingsNetwork& ttn) = 0;
    };

}
