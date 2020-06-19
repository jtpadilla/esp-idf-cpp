
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_event.h"
#include "LorawanLauncher.h"

const unsigned JOIN_RETRY_INTERVAL = 30;

namespace sc::lorawan
{

    LorawanLauncher::LorawanLauncher(const LorawanDriverPins& lorawanDriverPins, const LorawanParameters& lorawanParameters)
    {
        //lorawanDriver = sc::lorawan::LorawanDriver.singleton(lorawanDriverPins, lorawanParameters);
        lorawanDriver = nullptr;
    }

    LorawanLauncher::~LorawanLauncher()
    {
        delete lorawanDriver;
    }

    void LorawanLauncher::connect(ILorawanTaskFactory& lorawanTaskFactory) {

        // Se intenta el join de forma indefinida
        while (!lorawanDriver->join()) {
            printf("Joining...\n");
            vTaskDelay(JOIN_RETRY_INTERVAL * 1000 / portTICK_PERIOD_MS);
        }

        // Ya estamo en la red, podemos empezar
        printf("Joined!\n");
        lorawanTaskFactory.createAndLaunch(lorawanDriver);

    }

}