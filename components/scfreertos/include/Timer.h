#define once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>

namespace scfreertos
{

	/**
	 * @brief Wrapper around the %FreeRTOS timer functions.
	 */
	class Timer {
		
	public:
		Timer(char* name, TickType_t period, UBaseType_t reload, void* data, void (*callback)(Timer* pTimer));
		virtual ~Timer();
		void changePeriod(TickType_t newPeriod, TickType_t blockTime = portMAX_DELAY);
		void* getData();
		const char* getName();
		TickType_t getPeriod();
		void reset(TickType_t blockTime = portMAX_DELAY);
		void start(TickType_t blockTime = portMAX_DELAY);
		void stop(TickType_t blockTime = portMAX_DELAY);

	private:
		TimerHandle_t timerHandle;
		TickType_t period;
		void (*callback)(Timer* pTimer);
		static void internalCallback(TimerHandle_t xTimer);

	};

}