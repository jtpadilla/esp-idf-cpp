#define once

#include <string>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace genielink::os
{
    
    class Semaphore {

		public:
			Semaphore(std::string owner = "<Unknown>");
			~Semaphore();
			void        give();
			void        give(uint32_t value);
			void        giveFromISR();
			void        setName(std::string name);
			bool        take(std::string owner = "<Unknown>");
			bool        take(uint32_t timeoutMs, std::string owner = "<Unknown>");
			std::string toString();
			uint32_t	wait(std::string owner = "<Unknown>");

		private:
			SemaphoreHandle_t m_semaphore;
			pthread_mutex_t   m_pthread_mutex;
			std::string       m_name;
			std::string       m_owner;
			uint32_t          m_value;
			bool              m_usePthreads;

    };

}