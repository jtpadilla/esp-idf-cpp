#pragma once

#include "LorawanDriver.h"

class ExampleLorawanTask {

    public:
        ExampleLorawanTask(genielink::lorawan::LorawanDriver *lorawanDriver);
        void launch();

    private:
        genielink::lorawan::LorawanDriver *lorawanDriver;
        void messageReceived(const uint8_t* message, size_t length, port_t port);

};