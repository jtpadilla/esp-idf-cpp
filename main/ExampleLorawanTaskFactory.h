#pragma once

#include "LorawanDriver.h"
#include "ExampleLorawanTask.h"

class ExampleLorawanTaskFactory final: public genielink::lorawan::ILorawanTaskFactory {

    public:

        virtual ~ExampleLorawanTaskFactory() override {
            if (task) {
                delete task;
            }
        }

        virtual void createAndLaunch(genielink::lorawan::LorawanDriver *lorawanDriver) override
        {
            task = new ExampleLorawanTask(lorawanDriver);
            task->launch();
        }

    private:
        ExampleLorawanTask *task;

};

