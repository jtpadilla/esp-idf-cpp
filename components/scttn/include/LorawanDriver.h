#pragma once

#include "Ttn.h"
#include "LorawanParameter.h"
#include "LorawanTaskFactory.h"

namespace sc::lorawan
{

    class LorawanDriver 
    {

        public:
            TtnDriver(const LorawanParameter& lorawanParameterArg);
            void connect(ITtnTaskFactory& ttnTaskFactory);

        private:
            LorawanParameter lorawanParameter;
            Ttn ttn;

    };

}
