#pragma once

#include "LorawanDriver.h"

namespace genielink::lorawan
{
    
    class ILorawanTaskFactory
    {
        public:
            virtual ~ILorawanTaskFactory();
            virtual void createAndLaunch(LorawanDriver *lorawanDriver) = 0;

    };

}
