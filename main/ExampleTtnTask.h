#pragma once

#include "TheThingsNetwork.h"

class ExampleTtnTask {

    public:
        ExampleTtnTask(TheThingsNetwork& ttn);
        void launch();

    private:
        TheThingsNetwork ttn;

        void txTask(void* pvParameter);
        void messageReceived(const uint8_t* message, size_t length, port_t port);


};