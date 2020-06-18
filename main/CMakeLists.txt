idf_component_register(
    SRCS 
        "mainFreeRtos.cpp" 
        "main.cpp" 

        "ExampleTtnTaskFactory.h"
        "ExampleTtnTask.h"
        "ExampleTtnTask.cpp"

        "system/System.cpp" 
        "system/System.h"
        "system/GeneralUtils.cpp" 
        "system/GeneralUtils.h"

        "freertos/Timer.cpp" 
        "freertos/Timer.h"
        "freertos/Task.cpp" 
        "freertos/Task.h"
        "freertos/Semaphore.cpp" 
        "freertos/Semaphore.h"
        "freertos/RingBuffer.cpp" 
        "freertos/RingBuffer.h"

        "ttn/TtnProvisioning.h" 
        "ttn/TtnDriver.h" 
        "ttn/TtnDriver.cpp" 
        "ttn/TtnTaskFactory.h"

    REQUIRES ttn-esp32
)