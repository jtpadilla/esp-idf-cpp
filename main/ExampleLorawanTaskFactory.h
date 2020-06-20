#pragma once

#include "LorawanDriver.h"
#include "ExampleLorawanTask.h"

class ExampleLorawanTaskFactory final: public sc::lorawan::ILorawanTaskFactory {

    public:

        virtual ~ExampleLorawanTaskFactory() override {
            if (task) {
                delete task;
            }
        }

        virtual void createAndLaunch(sc::lorawan::LorawanDriver *lorawanDriver) override
        {
            task = new ExampleLorawanTask(lorawanDriver);
            task->launch();
        }

    private:
        ExampleLorawanTask *task;

};

