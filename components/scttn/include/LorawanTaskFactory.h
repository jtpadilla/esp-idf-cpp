#pragma once

#include "Ttn.h"

namespace sc::lorawan
{
    
    class ILorawanTaskFactory
    {
        public:
            virtual void createAndLaunch(Ttn& ttn) = 0;
    };

}
