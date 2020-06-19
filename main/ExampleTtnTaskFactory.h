#pragma once

#include "LorawanDriver.h"
#include "ExampleLorawanTask.h"

class ExampleLorawanTaskFactory: public sc::lorawan::ILorawanTaskFactory {

    public:

        virtual ~ExampleLorawanTaskFactory() {
            if (task) {
                delete task;
            }
        }

        virtual void createAndLaunch(Ttn& ttn)
        {
            task = new ExampleLorawanTask(ttn);
            task->launch();
        }

    private:
        ExampleLorawanTask *task;

};

