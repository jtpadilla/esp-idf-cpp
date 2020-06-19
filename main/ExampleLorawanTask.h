#pragma once

#include "LorawanDriver.h"

class ExampleLorawanTask {

    public:
        ExampleLorawanTask(LorawanDriver *lorawanDriver);
        void launch();

    private:
        LorawanDriver *lorawanDriver;

        void txTask(void* pvParameter);
        void messageReceived(const uint8_t* message, size_t length, port_t port);


};