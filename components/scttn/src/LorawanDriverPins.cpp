
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
//#include "esp_event.h"

#include "LorawanDriverPins.h"

namespace sc::lorawan
{

    LorawanDriverPins::LorawanDriverPins(
                spi_host_device_t spi_host_arg,
                uint8_t spi_dma_chan_arg,
                uint8_t pin_spi_sclk_arg,
                uint8_t pin_spi_mosi_arg,
                uint8_t pin_spi_miso_arg,
                uint8_t pin_nss_arg,
                uint8_t pin_rxtx_arg,
                uint8_t pin_rst_arg,
                uint8_t pin_di00_arg,
                uint8_t pin_di01_arg):
                spi_host {spi_host_arg},
                spi_dma_chan {spi_dma_chan_arg},
                pin_spi_sclk {pin_spi_sclk_arg},
                pin_spi_mosi {pin_spi_mosi_arg},
                pin_spi_miso {pin_spi_miso_arg},
                pin_nss {pin_nss_arg},
                pin_rxtx {pin_rxtx_arg},
                pin_rst {pin_rst_arg},
                pin_di00 {pin_di00_arg},
                pin_di01 {pin_di01_arg}
    {

                esp_err_t err;
                
                // Initialize the GPIO ISR handler service
                err = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
                ESP_ERROR_CHECK(err);

                // Initialize SPI bus
                spi_bus_config_t spi_bus_config{};
                spi_bus_config.miso_io_num = pin_spi_miso;
                spi_bus_config.mosi_io_num = pin_spi_mosi;
                spi_bus_config.sclk_io_num = pin_spi_sclk;
                spi_bus_config.quadwp_io_num = -1;
                spi_bus_config.quadhd_io_num = -1;
                spi_bus_config.max_transfer_sz = 0;
                err = spi_bus_initialize(spi_host, &spi_bus_config, spi_dma_chan);
                ESP_ERROR_CHECK(err);

    }

    spi_host_device_t LorawanDriverPins::get_spi_host() const {
        return spi_host;
    }

    uint8_t LorawanDriverPins::get_spi_dma_chan() const {
        return spi_dma_chan;
    }

    uint8_t LorawanDriverPins::get_pin_spi_sclk() const {
        return pin_spi_sclk;
    }

    uint8_t LorawanDriverPins::get_pin_spi_mosi() const {
        return pin_spi_mosi;
    }

    uint8_t LorawanDriverPins::get_pin_spi_miso() const {
        return pin_spi_miso;
    }

    uint8_t LorawanDriverPins::get_pin_nss() const {
        return pin_nss;
    }

    uint8_t LorawanDriverPins::get_pin_rxtx() const {
        return pin_rxtx;
    }

    uint8_t LorawanDriverPins::get_pin_rst() const {
        return pin_rst;
    }

    uint8_t LorawanDriverPins::get_pin_di00() const {
        return pin_di00;
    }

    uint8_t LorawanDriverPins::get_pin_di01() const {
        return pin_di01;
    }


}