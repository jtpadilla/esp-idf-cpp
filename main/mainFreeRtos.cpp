
#include <stdio.h>
#include <Task.h>
#include "system/GeneralUtils.h"

class MyTask: public freertos::Task
{

	public:

		MyTask(std::string taskName, uint32_t millisecsParam):
			freertos::Task{taskName}, millisecs {millisecsParam}
		{
		}

		void run(void* data) {

			int count = 0;

			while(count++ < 10) {
				printf("[%s] count: %d\n", m_taskName.c_str(), count);
				delay(millisecs);
			}

			systemm::GeneralUtils::dumpInfo();
			printf("Done\n");

		}

	private:
		uint32_t millisecs;

};

void mainFreeRtos() {

	printf("Inicio TTN!\n");

	MyTask* pMyTask1 = new MyTask("uno", 1000);
	pMyTask1->setStackSize(20000);
	pMyTask1->start();

	MyTask* pMyTask2 = new MyTask("dossssssss", 2000);
	pMyTask2->setStackSize(20000);
	pMyTask2->start();

	printf("Final de la tarea principal!");

}
