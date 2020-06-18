#pragma once

#include "Ttn.h"
#include "TtnParameters.h"
#include "TtnTaskFactory.h"

namespace scttn
{
    class TtnDriver 
    {

        public:
            TtnDriver(const TtnParameters& ttnParametersParameter);
            void connect(ITtnTaskFactory& ttnTaskFactory);

        private:
            TtnParameters ttnParameters;
            Ttn ttn;

    };

}
