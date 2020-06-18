#pragma once

#include "TtnTaskFactory.h"
#include "ExampleTtnTask.h"

class ExampleTtnTaskFactory: public scttn::ITtnTaskFactory {

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

