#pragma once

#include <exception>
#include <string>

namespace sc::lorawan
{

    class LorawanParametersException: public std::exception
    {

        public:

            LorawanParametersException(std::string msgParam):
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