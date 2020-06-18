#pragma once

#include "Ttn.h"

namespace scttn
{
    class ITtnTaskFactory
    {
        public:
            virtual void createAndLaunch(Ttn& ttn) = 0;
    };

}
