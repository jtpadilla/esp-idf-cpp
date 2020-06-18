
#include "string"

#include "TtnProvisioning.h"
#include "TtnDriver.h"

#include "ExampleTtnTaskFactory.h"

constexpr char devEui[] = "004CFEED74AD2FA6";
constexpr char appEui[] = "70B3D57ED00306F7";
constexpr char appKey[] = "8214F6A2800C9FCD9B26BBE28D5CD057";

void mainTtn(void)
{

    // Se prepara la configuracion para conectar con TTN
    scttn::TtnProvisioning ttnProvisioning { devEui, appEui, appKey };

    // Se crea el driver para comunicar con la red LoraWan
    scttn::TtnDriver ttndriver {ttnProvisioning};

    // Se encargara de crear la tarea que se hara cargo de la session con la red.
    ExampleTtnTaskFactory exampleTtnTaskFactory{};

    // Se intenta conectar y cuando se consiga se crea y inicia la tarea
    ttndriver.connect(exampleTtnTaskFactory);

}
