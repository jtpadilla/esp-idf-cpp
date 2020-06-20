
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_timer.h>
#include "esp_log.h"

#include "../lmic/lmic.h"
#include "../hal/LorawanDriverHAL.h"

static const char* const TAG = "lorawandriver_hal";

////////////////////////////////////
// I/O
////////////////////////////////////

void hal_pin_rxtx(u1_t val)
{
    if (lorawandriver_hal.pinRxTx == LMIC_UNUSED_PIN)
        return;
    
    gpio_set_level(lorawandriver_hal.pinRxTx, val);
}

void hal_pin_rst(u1_t val)
{
    if (lorawandriver_hal.pinRst == LMIC_UNUSED_PIN)
        return;

    if (val == 0 || val == 1)
    { // drive pin
        gpio_set_level(lorawandriver_hal.pinRst, val);
        gpio_set_direction(lorawandriver_hal.pinRst, GPIO_MODE_OUTPUT);
    }
    else
    { // keep pin floating
        gpio_set_level(lorawandriver_hal.pinRst, val);
        gpio_set_direction(lorawandriver_hal.pinRst, GPIO_MODE_INPUT);
    }
}

s1_t hal_getRssiCal (void)
{
    return lorawandriver_hal.rssiCal;
}

ostime_t hal_setModuleActive (bit_t val)
{
    return 0;
}

bit_t hal_queryUsingTcxo(void)
{
    return false;
}

uint8_t hal_getTxPowerPolicy(u1_t inputPolicy, s1_t requestedPower, u4_t frequency)
{
    return LMICHAL_radio_tx_power_policy_paboost;
}


////////////////////////////////////
// SPI
////////////////////////////////////

void hal_spi_write(u1_t cmd, const u1_t *buf, size_t len)
{
    lorawandriver_hal.spiWrite(cmd, buf, len);
}

void hal_spi_read(u1_t cmd, u1_t *buf, size_t len)
{
    lorawandriver_hal.spiRead(cmd, buf, len);
}


////////////////////////////////////
// TIME
////////////////////////////////////

// Gets current time in LMIC ticks
u4_t hal_ticks()
{
    // LMIC tick unit: 16µs
    // esp_timer unit: 1µs
    return (u4_t)(esp_timer_get_time() >> 4);
}

// Wait until the specified time.
// Called if the LMIC code needs to wait for a precise time.
// All other events are ignored and will be served later.
u4_t hal_waitUntil(u4_t time)
{
    return lorawandriver_hal.waitUntil(time);
}

// Check if the specified time has been reached or almost reached.
// Otherwise, save it as alarm time.
// LMIC calls this function with the scheduled time of the next job
// in the queue. If the job is not due yet, LMIC will go to sleep.
u1_t hal_checkTimer(uint32_t time)
{
    return lorawandriver_hal.checkTimer(time);
}

// Go to sleep until next event.
// Called when LMIC is not busy and not job is due to be executed.
void hal_sleep()
{
    lorawandriver_hal.sleep();
}


////////////////////////////////////
// IRQ
////////////////////////////////////

void hal_disableIRQs()
{
    // nothing to do as interrupt handlers post message to queue
    // and don't access any shared data structures
}

void hal_enableIRQs()
{
    // nothing to do as interrupt handlers post message to queue
    // and don't access any shared data structures
}

void hal_init_ex(const void *pContext)
{
    lorawandriver_hal.init();
}


////////////////////////////////////
// Fatal failure
////////////////////////////////////

static hal_failure_handler_t* custom_hal_failure_handler = nullptr;

void hal_set_failure_handler(const hal_failure_handler_t* const handler)
{
    custom_hal_failure_handler = handler;
}

void hal_failed(const char *file, u2_t line)
{
    if (custom_hal_failure_handler != nullptr)
        (*custom_hal_failure_handler)(file, line);

    ESP_LOGE(TAG, "LMIC failed and stopped: %s:%d", file, line);

    // go to sleep forever
    while (true)
    {
        vTaskDelay(portMAX_DELAY);
    }
}
