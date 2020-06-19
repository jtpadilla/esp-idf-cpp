#pragma once

#include "Lorawan.h"
#include "LorawanParameter.h"
#include "LorawanTaskFactory.h"

namespace sc::lorawan
{

    class LorawanLauncher 
    {

        public:
            LorawanLauncher(const LorawanParameter& lorawanParameterArg);
            void connect(ILorawanTaskFactory& lorawanTaskFactoryArg);

        private:
            LorawanParameter lorawanParameter;
            Lorawan lorawan;

    };

}
