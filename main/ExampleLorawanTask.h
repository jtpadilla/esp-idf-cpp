#pragma once

#include "LorawanDriver.h"

class ExampleLorawanTask {

    public:
        ExampleLorawanTask(sc::lorawan::LorawanDriver *lorawanDriver);
        void launch();

    private:
        sc::lorawan::LorawanDriver *lorawanDriver;
        void messageReceived(const uint8_t* message, size_t length, port_t port);

};