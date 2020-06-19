#pragma once

#include "Lorawan.h"

namespace sc::lorawan
{
    
    class ILorawanTaskFactory
    {
        public:
            virtual void createAndLaunch(Lorawan& lorawan) = 0;
    };

}
