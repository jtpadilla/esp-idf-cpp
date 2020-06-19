
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ExampleLorawanTask.h"

ExampleLorawanTask::ExampleLorawanTask(sc::lorawan::LorawanDriver *lorawanDriverArg): 
    lorawanDriver{lorawanDriverArg}
{
}

void ExampleLorawanTask::launch() {

    printf("start: ExampleLorawanTask::launch(...)\n");      

    const unsigned TX_INTERVAL = 30;
    uint8_t msgData[] = "Hello, world";

    while (1) {
        printf("Sending message...\n");
        sc::lorawan::LorawanResponseCode res = lorawanDriver->transmitMessage(msgData, sizeof(msgData) - 1);
        printf(res == sc::lorawan::LorawanResponseCode.kTTNSuccessfulTransmission ? "Message sent.\n" : "Transmission failed.\n");

        vTaskDelay(TX_INTERVAL * 1000 / portTICK_PERIOD_MS);
    }

}


void ExampleLorawanTask::messageReceived(const uint8_t* message, size_t length, port_t port)
{
    printf("Message of %d bytes received on port %d:", length, port);
    for (int i = 0; i < length; i++)
        printf(" %02x", message[i]);
    printf("\n");
}

