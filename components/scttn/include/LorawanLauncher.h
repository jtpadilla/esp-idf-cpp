#pragma once

#include "Lorawan.h"
#include "LorawanParameter.h"
#include "LorawanTaskFactory.h"

namespace sc::lorawan
{

    class LorawanLauncher 
    {

        public:
            LorawanLauncher(const LorawanDriverPins& lorawanDriverPins, const LorawanParameter& lorawanParameter);
            ~LorawanLauncher();
            
            void connect(ILorawanTaskFactory& lorawanTaskFactoryArg);

        private:
            LorawanDriver *lorawanDriver;

    };

}
