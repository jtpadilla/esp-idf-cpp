#pragma once

#include "LorawanDriver.h"
#include "LorawanDriverPins.h"
#include "LorawanParameters.h"
#include "LorawanTaskFactory.h"

namespace genielink::lorawan
{

    class LorawanLauncher 
    {

        public:
            LorawanLauncher(const LorawanDriverPins& lorawanDriverPins, const LorawanParameters& lorawanParameters);
            ~LorawanLauncher();
            
            void connect(ILorawanTaskFactory& lorawanTaskFactoryArg);

        private:
            LorawanDriver *lorawanDriver;

    };

}
