#pragma once

#include "ttn/TtnTaskFactory.h"
#include "ExampleTtnTask.h"

class ExampleTtnTaskFactory: public speedycontrol::ttn::ITtnTaskFactory {

    public:

        virtual ~ExampleTtnTaskFactory() {
            if (task) {
                delete task;
            }
        }

        virtual void createAndLaunch(TheThingsNetwork& ttn)
        {
            task = new ExampleTtnTask(ttn);
            task->launch();
        }

    private:
        ExampleTtnTask *task;

};

