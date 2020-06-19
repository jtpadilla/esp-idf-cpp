#pragma once

#include "hal/hal_esp32.h"

#define NOT_CONNECTED 0xff

namespace sc::lorawan
{

    class LorawanDriverPins
    {
        public:

            const spi_host_device_t get_spi_host() const;
            const uint8_t get_spi_dma_chan() const;
            const uint8_t get_pin_spi_sclk() const;
            const uint8_t get_pin_spi_mosi() const;
            const uint8_t get_pin_spi_miso() const;
            const uint8_t get_pin_nss() const;
            const uint8_t get_pin_rxtx() const;
            const uint8_t get_pin_rst() const;
            const uint8_t get_pin_di00() const;
            const uint8_t get_pin_di01() const;

        protected:

            LorawanDriverPins(
                spi_host_device_t spi_host_arg
                uint8_t spi_dma_chan_arg,
                uint8_t pin_spi_sclk_arg,
                uint8_t pin_spi_mosi_arg,
                uint8_t pin_spi_miso_arg,
                uint8_t pin_nss_arg,
                uint8_t pin_rxtx_arg,
                uint8_t pin_rst_arg,
                uint8_t pin_di00_arg,
                uint8_t pin_di01_arg);

        private:

            const spi_host_device_t spi_host;
            const uint8_t spi_dma_chan;
            const uint8_t pin_spi_sclk;
            const uint8_t pin_spi_mosi;
            const uint8_t pin_spi_miso;
            const uint8_t pin_nss;
            const uint8_t pin_rxtx;
            const uint8_t pin_rst;
            const uint8_t pin_di00;
            const uint8_t pin_di01;

    }

}