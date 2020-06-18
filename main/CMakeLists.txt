idf_component_register(
    SRCS 
        "mainFreeRtos.cpp" 
        "mainTtn.cpp" 
        "main.cpp" 

        "ExampleTtnTaskFactory.h"
        "ExampleTtnTask.h"
        "ExampleTtnTask.cpp"

    REQUIRES scfreertos scsystem scttn
)