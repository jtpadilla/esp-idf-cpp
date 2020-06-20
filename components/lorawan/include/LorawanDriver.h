
#pragma once

#include <stdint.h>
#include "driver/spi_master.h"
#include "LorawanDriverPins.h"
#include "LorawanParameters.h"

typedef uint8_t port_t;

namespace genielink::lorawan
{

    /**
     * @brief Response codes
     */
    enum class LorawanResponseCode
    {
        ErrorTransmissionFailed = -1,
        ErrorUnexpected = -10,
        SuccessfulTransmission = 1,
        SuccessfulReceive = 2
    };

    /**
     * @brief Callback for recieved messages
     * 
     * @param payload  pointer to the received bytes
     * @param length   number of received bytes
     * @param port     port the message was received on
     */
    typedef void (*LorawanMessageCallback)(const uint8_t* payload, size_t length, port_t port);

    /**
     * @brief Lorawan device
     * 
     * The 'Lorawan' class enables ESP32 devices with SX1272/73/76/77/78/79 LoRaWAN chips
     * to communicate via The Things Network.
     * 
     * Only one instance of this class must be created.
     */
    class LorawanDriver
    {

        public:

            /**
             * @brief Crea la instancia del driver
             * 
             * @param lorawanDriverPinsArg configuracion de pins que utilizara el driver
             * @param lorawanParametersArg Parametros utilizados para la conexion con la red
             * 
             * Solo puede ser creada una instancia
             */
            static LorawanDriver *instantiate(const LorawanDriverPins lorawanDriverPinsArg, const LorawanParameters lorawanParametersArg);

            /**
             * @brief Construct a new The Things Network device instance.
             */
            LorawanDriver(const LorawanDriverPins lorawanDriverPinsArg);

            /**
             * @brief Destroy the The Things Network device instance.
             */
            ~LorawanDriver();

            /**
             * @brief Reset the LorawanDriver radio.
             * 
             * Does not clear provisioned keys.
             */
            void reset();

            /**
             * @brief Configures the pins used to communicate with the LoRaWAN radio chip.
             * 
             * 
             * The SPI bus must be first configured using spi_bus_initialize(). Then it is passed as the first parameter.
             * Additionally, 'gpio_install_isr_service()' must be called to initialize the GPIO ISR handler service.
             * 
             * @param spi_host  The SPI bus/peripherial to use (SPI_HOST, HSPI_HOST or VSPI_HOST).
             * @param nss       The GPIO pin number connected to the radio chip's NSS pin (serving as the SPI chip select)
             * @param rxtx      The GPIO pin number connected to the radio chip's RXTX pin (TTN_NOT_CONNECTED if not connected)
             * @param rst       The GPIO pin number connected to the radio chip's RST pin (TTN_NOT_CONNECTED if not connected)
             * @param dio0      The GPIO pin number connected to the radio chip's DIO0 pin
             * @param dio1      The GPIO pin number connected to the radio chip's DIO1 pin
             */
            void configurePins(spi_host_device_t spi_host, uint8_t nss, uint8_t rxtx, uint8_t rst, uint8_t dio0, uint8_t dio1);

            /**
             * @brief Activate the device via OTAA.
             * 
             * The app EUI, app key and dev EUI must already have been provisioned by a call to 'provision()'.
             * Before this function is called, 'nvs_flash_init' must have been called once.
             * 
             * The function blocks until the activation has completed or failed.
             * 
             * @return true   if the activation was succeful
             * @return false  if the activation failed
             */
            bool join();

            /**
             * @brief Transmit a message
             * 
             * The function blocks until the message could be transmitted and a message has been received
             * in the subsequent receive window (or the window expires). Additionally, the function will
             * first wait until the duty cycle allows a transmission (enforcing the duty cycle limits).
             * 
             * @param payload  bytes to be transmitted
             * @param length   number of bytes to be transmitted
             * @param port     port (default to 1)
             * @param confirm  flag indicating if a confirmation should be requested. Default to 'false'
             * @return TSuccessfulTransmission   Successful transmission
             * @return ErrorTransmissionFailed   Transmission failed
             * @return TErrorUnexpected          Unexpected error
             */
            LorawanResponseCode transmitMessage(const uint8_t *payload, size_t length, port_t port = 1, bool confirm = false);

            /**
             * @brief Set the function to be called when a message is received
             * 
             * When a message is received, the specified function is called. The
             * message, its length and the port number are provided as
             * parameters. The values are only valid during the duration of the
             * callback. So they must be immediately processed or copied.
             * 
             * Messages are received as a result of 'transmitMessage' or 'poll'. The callback is called
             * in the task that called any of these functions and it occurs before these functions
             * return control to the caller.
             * 
             * @param callback  the callback function
             */
            void onMessage(LorawanMessageCallback callback);

            /**
             * @brief Sets the RSSI calibration value for LBT (Listen Before Talk).
             * 
             * This value is added to RSSI measured prior to decision. It must include the guardband.
             * Ignored in US, EU, IN and other countries where LBT is not required.
             * Default to 10 dB.
             * 
             * @param rssiCal RSSI calibration value, in dB
             */
            void setRSSICal(int8_t rssiCal);

        private:
            LorawanMessageCallback messageCallback;

    };

}

