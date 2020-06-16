
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ExampleTtnTask.h"

ExampleTtnTask::ExampleTtnTask(TheThingsNetwork& ttnParam): 
    ttn{ttnParam}
{
}

void ExampleTtnTask::launch() {

    printf("start: TtnExampleTask::launch(...)\n");      

    // Se instala el listener de los mensajes desde la red
    //ttn.onMessage(messageReceived);

    // Se instalar la tarea que se encarga de transmitir los mensakes hacia la red  
    //TaskFunction_t t = static_cast<TaskFunction_t>(txTask);
    //xTaskCreate(t, "send_messages", 1024 * 4, (void* )0, 3, nullptr);

    txTask(nullptr);

}

const unsigned TX_INTERVAL = 30;
static uint8_t msgData[] = "Hello, world";

void ExampleTtnTask::txTask(void* pvParameter)
{
    while (1) {
        printf("Sending message...\n");
        TTNResponseCode res = ttn.transmitMessage(msgData, sizeof(msgData) - 1);
        printf(res == kTTNSuccessfulTransmission ? "Message sent.\n" : "Transmission failed.\n");

        vTaskDelay(TX_INTERVAL * 1000 / portTICK_PERIOD_MS);
    }
}

void ExampleTtnTask::messageReceived(const uint8_t* message, size_t length, port_t port)
{
    printf("Message of %d bytes received on port %d:", length, port);
    for (int i = 0; i < length; i++)
        printf(" %02x", message[i]);
    printf("\n");
}

