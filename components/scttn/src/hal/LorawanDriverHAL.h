#pragma once

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_timer.h>

#define LMIC_UNUSED_PIN 0xff

namespace genielink::lorawan::hal
{

    enum class WaitKind {
        CHECK_IO,
        WAIT_FOR_ANY_EVENT,
        WAIT_FOR_TIMER
    };

    class LorawanDriverHAL
    {

        public:
            LorawanDriverHAL();

            void configurePins(spi_host_device_t spi_host, uint8_t nss, uint8_t rxtx, uint8_t rst, uint8_t dio0, uint8_t dio1);
            void init();
            void startLMICTask();
            
            void wakeUp();
            void initCriticalSection();
            void enterCriticalSection();
            void leaveCriticalSection();

            void spiWrite(uint8_t cmd, const uint8_t *buf, size_t len);
            void spiRead(uint8_t cmd, uint8_t *buf, size_t len);
            uint8_t checkTimer(uint32_t osTime);
            void sleep();
            
            uint32_t waitUntil(uint32_t osTime);

            spi_host_device_t spiHost;
            gpio_num_t pinNSS;
            gpio_num_t pinRxTx;
            gpio_num_t pinRst;
            gpio_num_t pinDIO0;
            gpio_num_t pinDIO1;
            int8_t rssiCal;

        private:
            static void lmicBackgroundTask(void* pvParameter);
            static void dioIrqHandler(void* arg);
            static void timerCallback(void *arg);
            static int64_t osTimeToEspTime(int64_t espNow, uint32_t osTime);

            void ioInit();
            void spiInit();
            void timerInit();

            void setNextAlarm(int64_t time);
            void armTimer(int64_t espNow);
            void disarmTimer();
            bool wait(WaitKind waitKind);

            static TaskHandle_t lmicTask;
            static uint32_t dioInterruptTime;
            static uint8_t dioNum;

            spi_device_handle_t spiHandle;
            spi_transaction_t spiTransaction;
            SemaphoreHandle_t mutex;
            esp_timer_handle_t timer;
            int64_t nextAlarm;

    };

}

extern genielink::lorawan::hal::LorawanDriverHAL lorawandriver_hal;

