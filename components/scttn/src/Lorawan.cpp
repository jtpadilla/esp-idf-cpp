
#include "freertos/FreeRTOS.h"
#include "esp_event.h"
#include "esp_log.h"

#include "hal/hal_esp32.h"
#include "lmic/lmic.h"

#include "Lorawan.h"
#include "LorawanLogging.h"

namespace sc::lorawan
{

    /**
     * Motivo por el que el codigo del usuario esta esperando
     */
    enum TTNWaitingReason
    {
        eWaitingNone,
        eWaitingForJoin,
        eWaitingForTransmission
    };

    /**
     * Tipo de evento
     */
    enum TTNEvent {
        eEvtNone,
        eEvtJoinCompleted,
        eEvtJoinFailed,
        eEvtMessageReceived,
        eEvtTransmissionCompleted,
        eEvtTransmissionFailed
    };

    /**
     * Evento enviado desde LMIC a la tarea del cliente que esta esperando
     */
    struct TTNLmicEvent {

        TTNLmicEvent(TTNEvent ev = eEvtNone): event(ev) { }

        TTNEvent event;
        uint8_t port;
        const uint8_t* message;
        size_t messageSize;
        
    };

    static const char *TAG = "ttn";

    static Ttn* ttnInstance;
    static QueueHandle_t lmicEventQueue = nullptr;
    static TTNWaitingReason waitingReason = eWaitingNone;
    static TtnProvisioning provisioning;

    #if LMIC_ENABLE_event_logging
    static LorawanLogging* logging;
    #endif

    static void eventCallback(void* userData, ev_t event);
    static void messageReceivedCallback(void *userData, uint8_t port, const uint8_t *message, size_t messageSize);
    static void messageTransmittedCallback(void *userData, int success);

    // --- LMIC callbacks

    // This EUI must be in little-endian format, so least-significant-byte first.
    // When copying an EUI from ttnctl output, this means to reverse the bytes.
    // For TTN issued EUIs the last bytes should be 0xD5, 0xB3, 0x70.
    // The order is swapped in provisioning_decode_keys().
    void os_getArtEui (u1_t* buf)
    {
        memcpy(buf, global_app_eui, 8);
    }

    // This should also be in little endian format, see above.
    void os_getDevEui (u1_t* buf)
    {
        memcpy(buf, global_dev_eui, 8);
    }

    // This key should be in big endian format (or, since it is not really a number
    // but a block of memory, endianness does not really apply). In practice, a key
    // taken from ttnctl can be copied as-is.
    void os_getDevKey (u1_t* buf)
    {
        memcpy(buf, global_app_key, 16);
    }



    Lorawan::Lorawan()
        : messageCallback(nullptr)
    {
    #if defined(TTN_IS_DISABLED)
        ESP_LOGE(TAG, "TTN is disabled. Configure a frequency plan using 'make menuconfig'");
        ASSERT(0);
    #endif

        ASSERT(ttnInstance == nullptr);
        ttnInstance = this;
        ttn_hal.initCriticalSection();
    }

    Lorawan::~Lorawan()
    {
        // nothing to do
    }

    void Lorawan::configurePins(spi_host_device_t spi_host, uint8_t nss, uint8_t rxtx, uint8_t rst, uint8_t dio0, uint8_t dio1)
    {
        ttn_hal.configurePins(spi_host, nss, rxtx, rst, dio0, dio1);

    #if LMIC_ENABLE_event_logging
        logging = TTNLogging::initInstance();
    #endif

        LMIC_registerEventCb(eventCallback, nullptr);
        LMIC_registerRxMessageCb(messageReceivedCallback, nullptr);

        os_init_ex(nullptr);
        reset();

        lmicEventQueue = xQueueCreate(4, sizeof(TTNLmicEvent));
        ASSERT(lmicEventQueue != nullptr);
        ttn_hal.startLMICTask();
    }

    void Lorawan::reset()
    {
        ttn_hal.enterCriticalSection();
        LMIC_reset();
        waitingReason = eWaitingNone;
        if (lmicEventQueue != nullptr)
        {
            xQueueReset(lmicEventQueue);
        }
        ttn_hal.leaveCriticalSection();
    }

    bool Lorawan::provision(const char *devEui, const char *appEui, const char *appKey)
    {
        if (!provisioning.decodeKeys(devEui, appEui, appKey))
            return false;
        
    }

    bool Lorawan::provisionWithMAC(const char *appEui, const char *appKey)
    {
        if (!provisioning.fromMAC(appEui, appKey))
            return false;
        
    }


    void Lorawan::startProvisioningTask()
    {
    #if defined(TTN_HAS_AT_COMMANDS)
        provisioning.startTask();
    #else
        ESP_LOGE(TAG, "AT commands are disabled. Change the configuration using 'make menuconfig'");
        ASSERT(0);
        esp_restart();
    #endif
    }

    void Lorawan::waitForProvisioning()
    {
    #if defined(TTN_HAS_AT_COMMANDS)
        if (isProvisioned())
        {
            ESP_LOGI(TAG, "Device is already provisioned");
            return;
        }

        while (!provisioning.haveKeys())
            vTaskDelay(1000 / portTICK_PERIOD_MS);

        ESP_LOGI(TAG, "Device successfully provisioned");
    #else
        ESP_LOGE(TAG, "AT commands are disabled. Change the configuration using 'make menuconfig'");
        ASSERT(0);
        esp_restart();
    #endif
    }

    bool Lorawan::join(const char *devEui, const char *appEui, const char *appKey)
    {
        if (!provisioning.decodeKeys(devEui, appEui, appKey))
            return false;
        
        return joinCore();
    }

    bool Lorawan::join()
    {
        return joinCore();
    }

    bool Lorawan::joinCore()
    {
        ttn_hal.enterCriticalSection();
        waitingReason = eWaitingForJoin;
        LMIC_startJoining();
        ttn_hal.wakeUp();
        ttn_hal.leaveCriticalSection();

        TTNLmicEvent event;
        xQueueReceive(lmicEventQueue, &event, portMAX_DELAY);
        return event.event == eEvtJoinCompleted;
    }

    TTNResponseCode Lorawan::transmitMessage(const uint8_t *payload, size_t length, port_t port, bool confirm)
    {
        ttn_hal.enterCriticalSection();
        if (waitingReason != eWaitingNone || (LMIC.opmode & OP_TXRXPEND) != 0)
        {
            ttn_hal.leaveCriticalSection();
            return kTTNErrorTransmissionFailed;
        }

        waitingReason = eWaitingForTransmission;
        LMIC.client.txMessageCb = messageTransmittedCallback;
        LMIC.client.txMessageUserData = nullptr;
        LMIC_setTxData2(port, (xref2u1_t)payload, length, confirm);
        ttn_hal.wakeUp();
        ttn_hal.leaveCriticalSection();

        while (true)
        {
            TTNLmicEvent result;
            xQueueReceive(lmicEventQueue, &result, portMAX_DELAY);

            switch (result.event)
            {
                case eEvtMessageReceived:
                    if (messageCallback != nullptr)
                        messageCallback(result.message, result.messageSize, result.port);
                    break;

                case eEvtTransmissionCompleted:
                    return kTTNSuccessfulTransmission;

                case eEvtTransmissionFailed:
                    return kTTNErrorTransmissionFailed;

                default:
                    ASSERT(0);
            }
        }
    }

    void Lorawan::onMessage(TTNMessageCallback callback)
    {
        messageCallback = callback;
    }

    void Lorawan::setRSSICal(int8_t rssiCal)
    {
        ttn_hal.rssiCal = rssiCal;
    }


    // --- Callbacks ---

    #if CONFIG_LOG_DEFAULT_LEVEL >= 3 || LMIC_ENABLE_event_logging
    const char *eventNames[] = { LMIC_EVENT_NAME_TABLE__INIT };
    #endif


    // Called by LMIC when an LMIC event (join, join failed, reset etc.) occurs
    void eventCallback(void* userData, ev_t event)
    {
    #if LMIC_ENABLE_event_logging
        logging->logEvent(event, eventNames[event], 0);
    #elif CONFIG_LOG_DEFAULT_LEVEL >= 3
        ESP_LOGI(TAG, "event %s", eventNames[event]);
    #endif

        TTNEvent ttnEvent = eEvtNone;

        if (waitingReason == eWaitingForJoin)
        {
            if (event == EV_JOINED)
            {
                ttnEvent = eEvtJoinCompleted;
            }
            else if (event == EV_REJOIN_FAILED || event == EV_RESET)
            {
                ttnEvent = eEvtJoinFailed;
            }
        }

        if (ttnEvent == eEvtNone)
            return;

        TTNLmicEvent result(ttnEvent);
        waitingReason = eWaitingNone;
        xQueueSend(lmicEventQueue, &result, pdMS_TO_TICKS(100));
    }

    // Sera llamado por LMIC cuando se reciba un mensaje
    void messageReceivedCallback(void *userData, uint8_t port, const uint8_t *message, size_t nMessage)
    {
        TTNLmicEvent result(eEvtMessageReceived);
        result.port = port;
        result.message = message;
        result.messageSize = nMessage;
        xQueueSend(lmicEventQueue, &result, pdMS_TO_TICKS(100));
    }

    // sera llamado por LMIC cuando un mensaje se ha transmitido (o la transmision ha fallado)
    void messageTransmittedCallback(void *userData, int success)
    {
        waitingReason = eWaitingNone;
        TTNLmicEvent result(success ? eEvtTransmissionCompleted : eEvtTransmissionFailed);
        xQueueSend(lmicEventQueue, &result, pdMS_TO_TICKS(100));
    }

}