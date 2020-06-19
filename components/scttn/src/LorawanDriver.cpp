
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "TtnDriver.h"

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

    LorawanDriver::LorawanDriver(const LorawanParameter& lorawanParameterArg):
        lorawanParameter {lorawanParameterArg}
    {

        esp_err_t err;
        
        // Initialize the GPIO ISR handler service
        err = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
        ESP_ERROR_CHECK(err);

        // Initialize the NVS (non-volatile storage) for saving and restoring the keys
        err = nvs_flash_init();
        ESP_ERROR_CHECK(err);

        // Initialize SPI bus
        spi_bus_config_t spi_bus_config{};
        spi_bus_config.miso_io_num = TTN_PIN_SPI_MISO;
        spi_bus_config.mosi_io_num = TTN_PIN_SPI_MOSI;
        spi_bus_config.sclk_io_num = TTN_PIN_SPI_SCLK;
        spi_bus_config.quadwp_io_num = -1;
        spi_bus_config.quadhd_io_num = -1;
        spi_bus_config.max_transfer_sz = 0;
        err = spi_bus_initialize(TTN_SPI_HOST, &spi_bus_config, TTN_SPI_DMA_CHAN);
        ESP_ERROR_CHECK(err);

        // Configure the SX127x pins
        ttn.configurePins(TTN_SPI_HOST, TTN_PIN_NSS, TTN_PIN_RXTX, TTN_PIN_RST, TTN_PIN_DIO0, TTN_PIN_DIO1);

        // The below line can be commented after the first run as the data is saved in NVS
        ttn.provision(lorawanParameter.getDevEui(), ttnParameters.getAppEui(), ttnParameters.getAppKey());

    }

    void LorawanDriver::connect(ITtnTaskFactory& ttnTaskFacyoty) {

        // Se intenta el join de forma indefinida
        while (!ttn.join()) {
            printf("Joining...\n");
            vTaskDelay(JOIN_RETRY_INTERVAL * 1000 / portTICK_PERIOD_MS);
        }

        // Ya estamo en la red, podemos empezar
        printf("Joined!\n");
        ttnTaskFacyoty.createAndLaunch(ttn);

    }

}