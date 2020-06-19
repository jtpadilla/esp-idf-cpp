
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_event.h"
#include "LorawanLauncher.h"

/**
 * @brief Constant for indicating that a pin is not connected
 */
#define TTN_NOT_CONNECTED 0xff

// Pins PARA TTGO T-Beam 
#define TTN_SPI_HOST      HSPI_HOST
#define TTN_SPI_DMA_CHAN  1
#define TTN_PIN_SPI_SCLK  5
#define TTN_PIN_SPI_MOSI  27
#define TTN_PIN_SPI_MISO  19
#define TTN_PIN_NSS       18
#define TTN_PIN_RXTX      TTN_NOT_CONNECTED
#define TTN_PIN_RST       23
#define TTN_PIN_DIO0      26
#define TTN_PIN_DIO1      33

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