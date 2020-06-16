#pragma once

#include "TheThingsNetwork.h"
#include "TtnProvisioning.h"
#include "TtnTaskFactory.h"

namespace speedycontrol::ttn
{
    class TtnDriver 
    {

        public:
            TtnDriver(const TtnProvisioning& ttnProvisioningParameter);
            void connect(ITtnTaskFactory& ttnTaskFactory);

        private:
            TtnProvisioning ttnProvisioning;
            TheThingsNetwork ttn;

    };

}
