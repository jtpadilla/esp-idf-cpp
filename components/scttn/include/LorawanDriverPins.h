#pragma once

#include "driver/spi_master.h"

#define NOT_CONNECTED 0xff

namespace sc::lorawan
{

    class LorawanDriverPins
    {
        public:
            spi_host_device_t get_spi_host() const;
            uint8_t get_spi_dma_chan() const;
            uint8_t get_pin_spi_sclk() const;
            uint8_t get_pin_spi_mosi() const;
            uint8_t get_pin_spi_miso() const;
            uint8_t get_pin_nss() const;
            uint8_t get_pin_rxtx() const;
            uint8_t get_pin_rst() const;
            uint8_t get_pin_di00() const;
            uint8_t get_pin_di01() const;

        protected:
            LorawanDriverPins(
                spi_host_device_t spi_host_arg,
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
            spi_host_device_t spi_host;
            uint8_t spi_dma_chan;
            uint8_t pin_spi_sclk;
            uint8_t pin_spi_mosi;
            uint8_t pin_spi_miso;
            uint8_t pin_nss;
            uint8_t pin_rxtx;
            uint8_t pin_rst;
            uint8_t pin_di00;
            uint8_t pin_di01;

    };

}