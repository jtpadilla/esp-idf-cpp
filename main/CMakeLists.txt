idf_component_register(
    SRCS 
        "mainFreeRtos.cpp" 
        "mainLorawan.cpp" 
        "main.cpp" 

        "ExampleLorawanTaskFactory.h"
        "ExampleLorawanTask.h"
        "ExampleLorawanTask.cpp"

    REQUIRES os sys lorawan
)