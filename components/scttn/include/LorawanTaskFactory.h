#pragma once

#include "LorawanDriver.h"

namespace sc::lorawan
{
    
    class ILorawanTaskFactory
    {
        public:
            virtual ~ILorawanTaskFactory();
            virtual void createAndLaunch(LorawanDriver *lorawanDriver) = 0;

    };

}
