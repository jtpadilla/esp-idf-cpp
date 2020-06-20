
#include <cstring>

#include "freertos/FreeRTOS.h"
#include "esp_event.h"
#include "esp_log.h"

#include "hal/hal_esp32.h"
#include "lmic/lmic.h"

#include "LorawanDriver.h"
#include "LorawanLogging.h"

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// Conexion con la libreria LMIC (INICIO)
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

// --- LMIC callbacks
static uint8_t global_app_eui[8];
static uint8_t global_app_key[16];
static uint8_t global_dev_eui[8];

// This EUI must be in little-endian format, so least-significant-byte first.
// When copying an EUI from ttnctl output, this means to reverse the bytes.
// For TTN issued EUIs the last bytes should be 0xD5, 0xB3, 0x70.
// The order is swapped in provisioning_decode_keys().
void os_getArtEui (u1_t* buf)
{
    memcpy(buf, global_app_eui, 8);
}

// This key should be in big endian format (or, since it is not really a number
// but a block of memory, endianness does not really apply). In practice, a key
// taken from ttnctl can be copied as-is.
void os_getDevKey (u1_t* buf)
{
    memcpy(buf, global_app_key, 16);
}

// This should also be in little endian format, see above.
void os_getDevEui (u1_t* buf)
{
    memcpy(buf, global_dev_eui, 8);
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// Conexion con la libreria LMIC (FINAL)
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

namespace sc::lorawan
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

        TTNLmicEvent(TTNEvent ev = TTNEvent::None): event(ev) { }

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

        // Configuracion de LMIC
        std::memcpy(global_app_eui, &lorawanParameters.appEui, sizeof(lorawanParameters.appEui));
        std::memcpy(global_app_key, &lorawanParameters.appKey, sizeof(lorawanParameters.appKey));
        std::memcpy(global_dev_eui, &lorawanParameters.devEui, sizeof(lorawanParameters.devEui));

        return new LorawanDriver(lorawanDriverPins);

    }


    LorawanDriver::LorawanDriver(const LorawanDriverPins lorawanDriverPinsArg)
        : lorawanDriverPins{lorawanDriverPinsArg}, messageCallback(nullptr)
    {
    #if defined(TTN_IS_DISABLED)
        ESP_LOGE(TAG, "TTN is disabled. Configure a frequency plan using 'make menuconfig'");
        ASSERT(0);
    #endif

        ASSERT(ttnInstance == nullptr);
        ttnInstance = this;
        ttn_hal.initCriticalSection();
    }

    LorawanDriver::~LorawanDriver()
    {
        // nothing to do
    }

    void LorawanDriver::configurePins(spi_host_device_t spi_host, uint8_t nss, uint8_t rxtx, uint8_t rst, uint8_t dio0, uint8_t dio1)
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

    void LorawanDriver::reset()
    {
        ttn_hal.enterCriticalSection();
        LMIC_reset();
        waitingReason = TTNWaitingReason::None;
        if (lmicEventQueue != nullptr)
        {
            xQueueReset(lmicEventQueue);
        }
        ttn_hal.leaveCriticalSection();
    }


    bool LorawanDriver::join()
    {
        ttn_hal.enterCriticalSection();
        waitingReason = TTNWaitingReason::ForJoin;
        LMIC_startJoining();
        ttn_hal.wakeUp();
        ttn_hal.leaveCriticalSection();

        TTNLmicEvent event;
        xQueueReceive(lmicEventQueue, &event, portMAX_DELAY);
        return event.event == TTNEvent::JoinCompleted;
    }

    LorawanResponseCode LorawanDriver::transmitMessage(const uint8_t *payload, size_t length, port_t port, bool confirm)
    {
        ttn_hal.enterCriticalSection();
        if (waitingReason != TTNWaitingReason::None || (LMIC.opmode & OP_TXRXPEND) != 0)
        {
            ttn_hal.leaveCriticalSection();
            return LorawanResponseCode::ErrorTransmissionFailed;
        }

        waitingReason = TTNWaitingReason::ForTransmission;
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