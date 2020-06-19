#pragma once

#include <exception>
#include <string>

namespace scttn
{

    class LorawanParameterException: public exception
    {

        public:

            LorawanParameterException(String msgParam):
                msg {param}

            {
            }

            virtual const char* what() const throw()
            {
                return param.c_string();
            }

        private:
            string msg;
    }


}