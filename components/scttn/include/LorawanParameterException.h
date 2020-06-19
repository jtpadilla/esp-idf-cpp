#pragma once

#include <exception>
#include <string>

namespace scttn
{

    class LorawanParameterException: public exception
    {

        public:

            LorawanParameterException(std::string msgParam):
                msg {msgParam}
            {
            }

            virtual const char* what() const throw()
            {
                return msg.c_str();
            }

        private:
            std::string msg;
            
    };


}