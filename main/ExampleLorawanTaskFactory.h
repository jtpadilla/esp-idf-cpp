#pragma once

#include "LorawanDriver.h"
#include "ExampleLorawanTask.h"

class ExampleLorawanTaskFactory: public sc::lorawan::ILorawanTaskFactory {

    public:

/*
        virtual ~ExampleLorawanTaskFactory() {
            if (task) {
                delete task;
            }
        }
        */

        virtual void createAndLaunch(sc::lorawan::LorawanDriver *lorawanDriver)
        {
            task = new ExampleLorawanTask(lorawanDriver);
            task->launch();
        }

    private:
        ExampleLorawanTask *task;

};

