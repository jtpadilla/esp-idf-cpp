#pragma once

#include <string>

namespace speedycontrol::ttn
{
    class TtnProvisioning
    {

        public:

            TtnProvisioning(const char devEuiParam[], const char appEuiParam[], const char appKeyParam[]):
                devEui {devEuiParam}, appEui {appEuiParam}, appKey {appKeyParam}
            {
            }

            const char *getDevEui() const {
                return devEui.c_str(); 
            }        

            const char *getAppEui() const {
                return appEui.c_str(); 
            }        

            const char *getAppKey() const {
                return appKey.c_str(); 
            }        


        private:
            const std::string devEui;
            const std::string appEui;
            const std::string appKey;

    };

}
