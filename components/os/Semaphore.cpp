#include <sstream>
#include <iomanip>
#include "esp_log.h"
#include <Semaphore.h>

static const char* LOG_TAG = "Semaphore";

namespace genielink::os
{

	/**
	 * @brief Wait for a semaphore to be released by trying to take it and
	 * then releasing it again.
	 * @param [in] owner A debug tag.
	 * @return The value associated with the semaphore.
	 */
	uint32_t Semaphore::wait(std::string owner) {
		ESP_LOGV(LOG_TAG, ">> wait: Semaphore waiting: %s for %s", toString().c_str(), owner.c_str());
		
		m_owner = owner;

		if (m_usePthreads) {
			pthread_mutex_lock(&m_pthread_mutex);
		} else {
			xSemaphoreTake(m_semaphore, portMAX_DELAY);
		}

		if (m_usePthreads) {
			pthread_mutex_unlock(&m_pthread_mutex);
		} else {
			xSemaphoreGive(m_semaphore);
		}

		ESP_LOGV(LOG_TAG, "<< wait: Semaphore released: %s", toString().c_str());
		return m_value;
	} // wait


	Semaphore::Semaphore(std::string name) {
		m_usePthreads = false;   	// Are we using pThreads or FreeRTOS?
		if (m_usePthreads) {
			pthread_mutex_init(&m_pthread_mutex, nullptr);
		} else {
			m_semaphore = xSemaphoreCreateBinary();
			xSemaphoreGive(m_semaphore);
		}

		m_name      = name;
		m_owner     = std::string("<N/A>");
		m_value     = 0;
	}


	Semaphore::~Semaphore() {
		if (m_usePthreads) {
			pthread_mutex_destroy(&m_pthread_mutex);
		} else {
			vSemaphoreDelete(m_semaphore);
		}
	}


	/**
	 * @brief Give a semaphore.
	 * The Semaphore is given.
	 */
	void Semaphore::give() {
		ESP_LOGV(LOG_TAG, "Semaphore giving: %s", toString().c_str());
		if (m_usePthreads) {
			pthread_mutex_unlock(&m_pthread_mutex);
		} else {
			xSemaphoreGive(m_semaphore);
		}

		m_owner = std::string("<N/A>");
	} // Semaphore::give


	/**
	 * @brief Give a semaphore.
	 * The Semaphore is given with an associated value.
	 * @param [in] value The value to associate with the semaphore.
	 */
	void Semaphore::give(uint32_t value) {
		m_value = value;
		give();
	} // give


	/**
	 * @brief Give a semaphore from an ISR.
	 */
	void Semaphore::giveFromISR() {
		BaseType_t higherPriorityTaskWoken;
		if (m_usePthreads) {
			assert(false);
		} else {
			xSemaphoreGiveFromISR(m_semaphore, &higherPriorityTaskWoken);
		}
	} // giveFromISR


	/**
	 * @brief Take a semaphore.
	 * Take a semaphore and wait indefinitely.
	 * @param [in] owner The new owner (for debugging)
	 * @return True if we took the semaphore.
	 */
	bool Semaphore::take(std::string owner) {
		ESP_LOGD(LOG_TAG, "Semaphore taking: %s for %s", toString().c_str(), owner.c_str());
		bool rc = false;
		if (m_usePthreads) {
			pthread_mutex_lock(&m_pthread_mutex);
		} else {
			rc = ::xSemaphoreTake(m_semaphore, portMAX_DELAY) == pdTRUE;
		}
		m_owner = owner;
		if (rc) {
			ESP_LOGD(LOG_TAG, "Semaphore taken:  %s", toString().c_str());
		} else {
			ESP_LOGE(LOG_TAG, "Semaphore NOT taken:  %s", toString().c_str());
		}
		return rc;
	} // Semaphore::take


	/**
	 * @brief Take a semaphore.
	 * Take a semaphore but return if we haven't obtained it in the given period of milliseconds.
	 * @param [in] timeoutMs Timeout in milliseconds.
	 * @param [in] owner The new owner (for debugging)
	 * @return True if we took the semaphore.
	 */
	bool Semaphore::take(uint32_t timeoutMs, std::string owner) {
		ESP_LOGV(LOG_TAG, "Semaphore taking: %s for %s", toString().c_str(), owner.c_str());
		bool rc = false;
		if (m_usePthreads) {
			assert(false);  // We apparently don't have a timed wait for pthreads.
		} else {
			rc = ::xSemaphoreTake(m_semaphore, timeoutMs / portTICK_PERIOD_MS) == pdTRUE;
		}
		m_owner = owner;
		if (rc) {
			ESP_LOGV(LOG_TAG, "Semaphore taken:  %s", toString().c_str());
		} else {
			ESP_LOGE(LOG_TAG, "Semaphore NOT taken:  %s", toString().c_str());
		}
		return rc;
	} // Semaphore::take



	/**
	 * @brief Create a string representation of the semaphore.
	 * @return A string representation of the semaphore.
	 */
	std::string Semaphore::toString() {
		std::stringstream stringStream;
		stringStream << "name: "<< m_name << " (0x" << std::hex << std::setfill('0') << (uint32_t)m_semaphore << "), owner: " << m_owner;
		return stringStream.str();
	} // toString


	/**
	 * @brief Set the name of the semaphore.
	 * @param [in] name The name of the semaphore.
	 */
	void Semaphore::setName(std::string name) {
		m_name = name;
	} // setName

}