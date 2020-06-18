#pragma once

#include "Ttn.h"
#include "ExampleTtnTask.h"

class ExampleTtnTaskFactory: public scttn::ITtnTaskFactory {

    public:

        virtual ~ExampleTtnTaskFactory() {
            if (task) {
                delete task;
            }
        }

        virtual void createAndLaunch(Ttn& ttn)
        {
            task = new ExampleTtnTask(ttn);
            task->launch();
        }

    private:
        ExampleTtnTask *task;

};

