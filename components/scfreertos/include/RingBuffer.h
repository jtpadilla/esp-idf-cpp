#define once
	
#include <freertos/FreeRTOS.h>
#include "freertos/ringbuf.h"

namespace scfreertos
{

    class Ringbuffer {

        public:
            Ringbuffer(size_t length, RingbufferType_t type = RINGBUF_TYPE_NOSPLIT);
            ~Ringbuffer();

            void*    receive(size_t* size, TickType_t wait = portMAX_DELAY);
            void     returnItem(void* item);
            bool     send(void* data, size_t length, TickType_t wait = portMAX_DELAY);

        private:
            RingbufHandle_t m_handle;
            
	};

}