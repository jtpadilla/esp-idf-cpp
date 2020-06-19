
#include "string"

#include "LorawanDriverPinsTtgoTBeam.h"
#include "LorawanParameters.h"
#include "LorawanParametersUtil.h"
#include "LorawanDriver.h"
#include "LorawanLauncher.h"

#include "ExampleLorawanTaskFactory.h"

constexpr char devEui[] = "004CFEED74AD2FA6";
constexpr char appEui[] = "70B3D57ED00306F7";
constexpr char appKey[] = "8214F6A2800C9FCD9B26BBE28D5CD057";

void mainTtn(void)
{

    // Configuracion hardware para un TTG-T-Beam
    sc::lorawan::LorawanDriverPinsTtgoTBeam lorawanDriverPins {};    

    // Se prepara la configuracion para conectar con la red Lorawan
    sc::lorawan::LorawanParameters lorawanParameters = sc::lorawan::LorawanParametersUtil::convert(std::string{devEui}, std::string{appEui}, std::string{appKey});

    // Se crea el driver para comunicar con la red LoraWan
    sc::lorawan::LorawanLauncher lorawanLauncher {lorawanDriverPins, lorawanParameters};

    // Se encargara de crear la tarea que se hara cargo de la session con la red.
    ExampleLorawanTaskFactory exampleLorawanTaskFactory{};

    // Se intenta conectar y cuando se consiga se crea y inicia la tarea
    lorawanLauncher.connect(exampleLorawanTaskFactory);

}
