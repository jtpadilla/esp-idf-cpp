
#include "../lmic/lmic.h"
#include "../hal/LorawanDriverHAL.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/timer.h"
#include "esp_log.h"

static const char* const TAG = "lorawandriver_hal";

sc::lorawan::hal::LorawanDriverHAL lorawandriver_hal;

#define NOTIFY_BIT_DIO 1
#define NOTIFY_BIT_TIMER 2
#define NOTIFY_BIT_WAKEUP 4

namespace sc::lorawan::hal
{

    TaskHandle_t LorawanDriverHAL::lmicTask = nullptr;
    uint32_t LorawanDriverHAL::dioInterruptTime = 0;
    uint8_t LorawanDriverHAL::dioNum = 0;

    // -----------------------------------------------------------------------------
    // Constructor

    LorawanDriverHAL::LorawanDriverHAL()
        : rssiCal(10), nextAlarm(0)
    {    
    }

    // -----------------------------------------------------------------------------
    // I/O

    void LorawanDriverHAL::configurePins(spi_host_device_t spi_host, uint8_t nss, uint8_t rxtx, uint8_t rst, uint8_t dio0, uint8_t dio1)
    {
        spiHost = spi_host;
        pinNSS = (gpio_num_t)nss;
        pinRxTx = (gpio_num_t)rxtx;
        pinRst = (gpio_num_t)rst;
        pinDIO0 = (gpio_num_t)dio0;
        pinDIO1 = (gpio_num_t)dio1;

        // Until the background process has been started, use the current task
        // for supporting calls like `hal_waitUntil()`.
        lmicTask = xTaskGetCurrentTaskHandle();
    }


    void IRAM_ATTR LorawanDriverHAL::dioIrqHandler(void *arg)
    {
        dioInterruptTime = hal_ticks();
        dioNum = (u1_t)(long)arg;
        BaseType_t higherPrioTaskWoken = pdFALSE;
        xTaskNotifyFromISR(lmicTask, NOTIFY_BIT_DIO, eSetBits, &higherPrioTaskWoken);
        if (higherPrioTaskWoken)
            portYIELD_FROM_ISR();
    }

    void LorawanDriverHAL::ioInit()
    {
        // pinNSS and pinDIO0 and pinDIO1 are required
        ASSERT(pinNSS != LMIC_UNUSED_PIN);
        ASSERT(pinDIO0 != LMIC_UNUSED_PIN);
        ASSERT(pinDIO1 != LMIC_UNUSED_PIN);

        gpio_pad_select_gpio(pinNSS);
        gpio_set_level(pinNSS, 0);
        gpio_set_direction(pinNSS, GPIO_MODE_OUTPUT);

        if (pinRxTx != LMIC_UNUSED_PIN)
        {
            gpio_pad_select_gpio(pinRxTx);
            gpio_set_level(pinRxTx, 0);
            gpio_set_direction(pinRxTx, GPIO_MODE_OUTPUT);
        }

        if (pinRst != LMIC_UNUSED_PIN)
        {
            gpio_pad_select_gpio(pinRst);
            gpio_set_level(pinRst, 0);
            gpio_set_direction(pinRst, GPIO_MODE_OUTPUT);
        }

        // DIO pins with interrupt handlers
        gpio_pad_select_gpio(pinDIO0);
        gpio_set_direction(pinDIO0, GPIO_MODE_INPUT);
        gpio_set_intr_type(pinDIO0, GPIO_INTR_POSEDGE);

        gpio_pad_select_gpio(pinDIO1);
        gpio_set_direction(pinDIO1, GPIO_MODE_INPUT);
        gpio_set_intr_type(pinDIO1, GPIO_INTR_POSEDGE);

        ESP_LOGI(TAG, "IO initialized");
    }


    // -----------------------------------------------------------------------------
    // SPI

    void LorawanDriverHAL::spiInit()
    {
        // init device
        spi_device_interface_config_t spiConfig;
        memset(&spiConfig, 0, sizeof(spiConfig));
        spiConfig.mode = 1;
        spiConfig.clock_speed_hz = CONFIG_TTN_SPI_FREQ;
        spiConfig.command_bits = 0;
        spiConfig.address_bits = 8;
        spiConfig.spics_io_num = pinNSS;
        spiConfig.queue_size = 1;
        spiConfig.cs_ena_posttrans = 2;

        esp_err_t ret = spi_bus_add_device(spiHost, &spiConfig, &spiHandle);
        ESP_ERROR_CHECK(ret);

        ESP_LOGI(TAG, "SPI initialized");
    }

    void LorawanDriverHAL::spiWrite(uint8_t cmd, const uint8_t *buf, size_t len)
    {
        memset(&spiTransaction, 0, sizeof(spiTransaction));
        spiTransaction.addr = cmd;
        spiTransaction.length = 8 * len;
        spiTransaction.tx_buffer = buf;
        esp_err_t err = spi_device_transmit(spiHandle, &spiTransaction);
        ESP_ERROR_CHECK(err);
    }

    void LorawanDriverHAL::spiRead(uint8_t cmd, uint8_t *buf, size_t len)
    {
        memset(buf, 0, len);
        memset(&spiTransaction, 0, sizeof(spiTransaction));
        spiTransaction.addr = cmd;
        spiTransaction.length = 8 * len;
        spiTransaction.rxlength = 8 * len;
        spiTransaction.tx_buffer = buf;
        spiTransaction.rx_buffer = buf;
        esp_err_t err = spi_device_transmit(spiHandle, &spiTransaction);
        ESP_ERROR_CHECK(err);
    }

    // -----------------------------------------------------------------------------
    // TIME

    /*
    * LIMIC uses a 32 bit time system (ostime_t) counting ticks. In this
    * implementation each tick is 16µs. It will wrap arounnd every 19 hours.
    * 
    * The ESP32 has a 64 bit timer counting microseconds. It will wrap around
    * every 584,000 years. So we don't need to bother.
    * 
    * Based on this timer, future callbacks can be scheduled. This is used to
    * schedule the next LMIC job.
    */

    // Convert LMIC tick time (ostime_t) to ESP absolute time.
    // `osTime` is assumed to be somewhere between one hour in the past and
    // 18 hours into the future. 
    int64_t LorawanDriverHAL::osTimeToEspTime(int64_t espNow, uint32_t osTime)
    {
        int64_t espTime;
        uint32_t osNow = (uint32_t)(espNow >> 4);

        // unsigned difference:
        // 0x00000000 - 0xefffffff: future (0 to about 18 hours)
        // 0xf0000000 - 0xffffffff: past (about 1 to 0 hours)
        uint32_t osDiff = osTime - osNow;
        if (osDiff < 0xf0000000)
        {
            espTime = espNow + (((int64_t)osDiff) << 4);
        }
        else
        {
            // one's complement instead of two's complement:
            // off by 1 µs and ignored
            osDiff = ~osDiff;
            espTime = espNow - (((int64_t)osDiff) << 4);
        }

        return espTime;
    }

    void LorawanDriverHAL::timerInit()
    {
        esp_timer_create_args_t timerConfig = {
            .callback = &timerCallback,
            .arg = nullptr,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "lmic_job"
        };
        esp_err_t err = esp_timer_create(&timerConfig, &timer);
        ESP_ERROR_CHECK(err);

        ESP_LOGI(TAG, "Timer initialized");
    }

    void LorawanDriverHAL::setNextAlarm(int64_t time)
    {
        nextAlarm = time;
    }

    void LorawanDriverHAL::armTimer(int64_t espNow)
    {
        if (nextAlarm == 0)
            return;
        int64_t timeout = nextAlarm - esp_timer_get_time();
        if (timeout < 0)
            timeout = 10;
        esp_timer_start_once(timer, timeout);
    }

    void LorawanDriverHAL::disarmTimer()
    {
        esp_timer_stop(timer);
    }

    void LorawanDriverHAL::timerCallback(void *arg)
    {
        xTaskNotify(lmicTask, NOTIFY_BIT_TIMER, eSetBits);
    }

    // Wait for the next external event. Either:
    // - scheduled timer due to scheduled job or waiting for a given time
    // - wake up event from the client code
    // - I/O interrupt (DIO0 or DIO1 pin)
    bool LorawanDriverHAL::wait(WaitKind waitKind)
    {
        TickType_t ticksToWait = waitKind == WaitKind::CHECK_IO ? 0 : portMAX_DELAY;
        while (true)
        {
            uint32_t bits = ulTaskNotifyTake(pdTRUE, ticksToWait);
            if (bits == 0)
                return false;

            if ((bits & NOTIFY_BIT_WAKEUP) != 0)
            {
                if (waitKind != WaitKind::WAIT_FOR_TIMER)
                {
                    disarmTimer();
                    return true;
                }
            }
            else if ((bits & NOTIFY_BIT_TIMER) != 0)
            {
                disarmTimer();
                setNextAlarm(0);
                if (waitKind != WaitKind::CHECK_IO)
                    return true;
            }
            else // IO interrupt
            {
                if (waitKind != WaitKind::WAIT_FOR_TIMER)
                    disarmTimer();
                enterCriticalSection();
                radio_irq_handler_v2(dioNum, dioInterruptTime);
                leaveCriticalSection();
                if (waitKind != WaitKind::WAIT_FOR_TIMER)
                    return true;
            }
        }
    }


    uint32_t LorawanDriverHAL::waitUntil(uint32_t osTime)
    {
        int64_t espNow = esp_timer_get_time();
        int64_t espTime = osTimeToEspTime(espNow, osTime);
        setNextAlarm(espTime);
        armTimer(espNow);
        wait(WaitKind::WAIT_FOR_TIMER);

        u4_t osNow = hal_ticks();
        u4_t diff = osNow - osTime;
        return diff < 0x80000000U ? diff : 0;
    }

    // Called by client code to wake up LMIC to do something,
    // e.g. send a submitted messages.
    void LorawanDriverHAL::wakeUp()
    {
        xTaskNotify(lmicTask, NOTIFY_BIT_WAKEUP, eSetBits);
    }

    uint8_t LorawanDriverHAL::checkTimer(u4_t osTime)
    {
        int64_t espNow = esp_timer_get_time();
        int64_t espTime = osTimeToEspTime(espNow, osTime);
        int64_t diff = espTime - espNow;
        if (diff < 100)
            return 1; // timer has expired or will expire very soon

        setNextAlarm(espTime);
        return 0;
    }

    void LorawanDriverHAL::sleep()
    {
        if (wait(WaitKind::CHECK_IO))
            return;

        armTimer(esp_timer_get_time());
        wait(WaitKind::WAIT_FOR_ANY_EVENT);
    }


    // -----------------------------------------------------------------------------
    // IRQ


    // -----------------------------------------------------------------------------
    // Synchronization between application code and background task

    void LorawanDriverHAL::initCriticalSection()
    {
        mutex = xSemaphoreCreateRecursiveMutex();
    }

    void LorawanDriverHAL::enterCriticalSection()
    {
        xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    }

    void LorawanDriverHAL::leaveCriticalSection()
    {
        xSemaphoreGiveRecursive(mutex);
    }

    // -----------------------------------------------------------------------------

    void LorawanDriverHAL::lmicBackgroundTask(void* pvParameter) {
        os_runloop();
    }

    void LorawanDriverHAL::init()
    {
        // configure radio I/O and interrupt handler
        ioInit();
        // configure radio SPI
        spiInit();
        // configure timer and alarm callback
        timerInit();
    }

    void LorawanDriverHAL::startLMICTask() {
        xTaskCreate(lmicBackgroundTask, "ttn_lmic", 1024 * 4, nullptr, CONFIG_TTN_BG_TASK_PRIO, &lmicTask);

        // enable interrupts
        gpio_isr_handler_add(pinDIO0, dioIrqHandler, (void *)0);
        gpio_isr_handler_add(pinDIO1, dioIrqHandler, (void *)1);
    }

}