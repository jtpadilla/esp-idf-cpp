idf_component_register(
    SRCS 
        "main.cpp" 
        "ExampleTtnTaskFactory.h"
        "ExampleTtnTask.h"
        "ExampleTtnTask.cpp"
        "ttn/TtnProvisioning.h" 
        "ttn/TtnDriver.h" 
        "ttn/TtnDriver.cpp" 
        "ttn/TtnTaskFactory.h"
    REQUIRES ttn-esp32
)