
#include <cstring>

#include "freertos/FreeRTOS.h"
#include "esp_event.h"
#include "esp_log.h"

#include "hal/lmic_plugin.h"
#include "hal/LorawanDriverHAL.h"
#include "lmic/lmic.h"

#include "LorawanDriver.h"
#include "LorawanLogging.h"


namespace genielink::lorawan
{

    /**
     * Motivo por el que el codigo del usuario esta esperando
     */
    enum class TTNWaitingReason
    {
        None,
        ForJoin,
        ForTransmission
    };

    /**
     * Tipo de evento
     */
    enum class TTNEvent {
        None,
        JoinCompleted,
        JoinFailed,
        MessageReceived,
        TransmissionCompleted,
        TransmissionFailed
    };

    /**
     * Evento enviado desde LMIC a la tarea del cliente que esta esperando
     */
    struct TTNLmicEvent {

        TTNLmicEvent(TTNEvent ev = TTNEvent::None): event(ev) {
        }

        TTNEvent event;
        uint8_t port;
        const uint8_t* message;
        size_t messageSize;
        
    };

    static const char *TAG = "ttn";

    static LorawanDriver* ttnInstance;
    static QueueHandle_t lmicEventQueue = nullptr;
    static TTNWaitingReason waitingReason = TTNWaitingReason::None;

    #if LMIC_ENABLE_event_logging
    static LorawanLogging* logging;
    #endif

    static void eventCallback(void* userData, ev_t event);
    static void messageReceivedCallback(void *userData, uint8_t port, const uint8_t *message, size_t messageSize);
    static void messageTransmittedCallback(void *userData, int success);

    LorawanDriver* LorawanDriver::instantiate(const LorawanDriverPins lorawanDriverPins, const LorawanParameters lorawanParameters) {

        // Los parametros para conectarse a la red Lora se colocan en las variables golables
        // que utilizara LMIC
        std::memcpy(global_app_eui, &lorawanParameters.appEui, sizeof(lorawanParameters.appEui));
        std::memcpy(global_app_key, &lorawanParameters.appKey, sizeof(lorawanParameters.appKey));
        std::memcpy(global_dev_eui, &lorawanParameters.devEui, sizeof(lorawanParameters.devEui));

        return new LorawanDriver(lorawanDriverPins);

    }

    LorawanDriver::LorawanDriver(const LorawanDriverPins lorawanDriverPinsArg)
    {
    #if defined(TTN_IS_DISABLED)
        ESP_LOGE(TAG, "TTN is disabled. Configure a frequency plan using 'make menuconfig'");
        ASSERT(0);
    #endif

        ASSERT(ttnInstance == nullptr);
        ttnInstance = this;
        lorawandriver_hal.initCriticalSection();

       
        // The SPI bus must be first configured using spi_bus_initialize(). Then it is passed as the first parameter.
        // Additionally, 'gpio_install_isr_service()' must be called to initialize the GPIO ISR handler service.
        lorawandriver_hal.configurePins(
            lorawanDriverPinsArg.get_spi_host(), 
            lorawanDriverPinsArg.get_pin_nss(), 
            lorawanDriverPinsArg.get_pin_rxtx(), 
            lorawanDriverPinsArg.get_pin_rst(), 
            lorawanDriverPinsArg.get_pin_di00(), 
            lorawanDriverPinsArg.get_pin_di01()
            );

    #if LMIC_ENABLE_event_logging
        logging = TTNLogging::initInstance();
    #endif

        LMIC_registerEventCb(eventCallback, nullptr);
        LMIC_registerRxMessageCb(messageReceivedCallback, nullptr);

        os_init_ex(nullptr);
        reset();

        lmicEventQueue = xQueueCreate(4, sizeof(TTNLmicEvent));
        ASSERT(lmicEventQueue != nullptr);
        lorawandriver_hal.startLMICTask();

        
    }

    LorawanDriver::~LorawanDriver()
    {
        // nothing to do
    }

    void LorawanDriver::reset()
    {
        lorawandriver_hal.enterCriticalSection();
        LMIC_reset();
        waitingReason = TTNWaitingReason::None;
        if (lmicEventQueue != nullptr)
        {
            xQueueReset(lmicEventQueue);
        }
        lorawandriver_hal.leaveCriticalSection();
    }


    bool LorawanDriver::join()
    {
        lorawandriver_hal.enterCriticalSection();
        waitingReason = TTNWaitingReason::ForJoin;
        LMIC_startJoining();
        lorawandriver_hal.wakeUp();
        lorawandriver_hal.leaveCriticalSection();

        TTNLmicEvent event;
        xQueueReceive(lmicEventQueue, &event, portMAX_DELAY);
        return event.event == TTNEvent::JoinCompleted;
    }

    LorawanResponseCode LorawanDriver::transmitMessage(const uint8_t *payload, size_t length, port_t port, bool confirm)
    {
        lorawandriver_hal.enterCriticalSection();
        if (waitingReason != TTNWaitingReason::None || (LMIC.opmode & OP_TXRXPEND) != 0)
        {
            lorawandriver_hal.leaveCriticalSection();
            return LorawanResponseCode::ErrorTransmissionFailed;
        }

        waitingReason = TTNWaitingReason::ForTransmission;
        LMIC.client.txMessageCb = messageTransmittedCallback;
        LMIC.client.txMessageUserData = nullptr;
        LMIC_setTxData2(port, (xref2u1_t)payload, length, confirm);
        lorawandriver_hal.wakeUp();
        lorawandriver_hal.leaveCriticalSection();

        while (true)
        {
            TTNLmicEvent result;
            xQueueReceive(lmicEventQueue, &result, portMAX_DELAY);

            switch (result.event)
            {
                case TTNEvent::MessageReceived:
                    if (messageCallback != nullptr)
                        messageCallback(result.message, result.messageSize, result.port);
                    break;

                case TTNEvent::TransmissionCompleted:
                    return LorawanResponseCode::SuccessfulTransmission;

                case TTNEvent::TransmissionFailed:
                    return LorawanResponseCode::ErrorTransmissionFailed;

                default:
                    ASSERT(0);
            }
        }
    }

    void LorawanDriver::onMessage(LorawanMessageCallback callback)
    {
        messageCallback = callback;
    }

    void LorawanDriver::setRSSICal(int8_t rssiCal)
    {
        lorawandriver_hal.rssiCal = rssiCal;
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

        TTNEvent ttnEvent = TTNEvent::None;

        if (waitingReason == TTNWaitingReason::ForJoin)
        {
            if (event == EV_JOINED)
            {
                ttnEvent = TTNEvent::JoinCompleted;
            }
            else if (event == EV_REJOIN_FAILED || event == EV_RESET)
            {
                ttnEvent = TTNEvent::JoinFailed;
            }
        }

        if (ttnEvent == TTNEvent::None)
            return;

        TTNLmicEvent result(ttnEvent);
        waitingReason = TTNWaitingReason::None;
        xQueueSend(lmicEventQueue, &result, pdMS_TO_TICKS(100));
    }

    // Sera llamado por LMIC cuando se reciba un mensaje
    void messageReceivedCallback(void *userData, uint8_t port, const uint8_t *message, size_t nMessage)
    {
        TTNLmicEvent result(TTNEvent::MessageReceived);
        result.port = port;
        result.message = message;
        result.messageSize = nMessage;
        xQueueSend(lmicEventQueue, &result, pdMS_TO_TICKS(100));
    }

    // sera llamado por LMIC cuando un mensaje se ha transmitido (o la transmision ha fallado)
    void messageTransmittedCallback(void *userData, int success)
    {
        waitingReason = TTNWaitingReason::None;
        TTNLmicEvent result(success ? TTNEvent::TransmissionCompleted : TTNEvent::TransmissionFailed);
        xQueueSend(lmicEventQueue, &result, pdMS_TO_TICKS(100));
    }

}