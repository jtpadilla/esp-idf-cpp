#pragma once

#include "Ttn.h"

class ExampleTtnTask {

    public:
        ExampleTtnTask(Ttn& ttn);
        void launch();

    private:
        Ttn ttn;

        void txTask(void* pvParameter);
        void messageReceived(const uint8_t* message, size_t length, port_t port);


};