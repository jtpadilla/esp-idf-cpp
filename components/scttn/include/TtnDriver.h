#pragma once

#include "Ttn.h"
#include "TtnProvisioning.h"
#include "TtnTaskFactory.h"

namespace scttn
{
    class TtnDriver 
    {

        public:
            TtnDriver(const TtnProvisioning& ttnProvisioningParameter);
            void connect(ITtnTaskFactory& ttnTaskFactory);

        private:
            TtnProvisioning ttnProvisioning;
            Ttn ttn;

    };

}
