#include <string>
#include "OS.h"
#include "OSException.h"
#include "cmsis_os2.h"

extern uint32_t SystemCoreClock;        /**< System Clock Frequency (Core Clock) */

namespace cmsis
{
	namespace os
	{
		std::string version()
		{
			char infobuf[100];
			osVersion_t osv;

			osStatus_t sta = osKernelGetInfo(&osv, infobuf, sizeof(infobuf));
			if(sta != osOK)
				throw std::system_error(cmsis::error_code(sta), "osKernelGetInfo");

			return std::string(infobuf);
		}

		uint32_t tick_frequency()
		{
			uint32_t tick = osKernelGetTickFreq();
			if (!tick)
				throw std::system_error(cmsis::error_code(osError), "osKernelGetTickFreq");

			return tick;
		}

		/**
		 * Initialize the RTOS Kernel.
		 * @throw std::system_error if an error occurs
		 */
		void initialize()
		{
			if (osKernelGetState() == osKernelInactive)
			{
				osStatus_t sta = osKernelInitialize();
				if (sta != osOK)
					throw std::system_error(cmsis::error_code(sta), "osKernelInitialize");
			}
		}

		/**
		 * Start the RTOS Kernel scheduler. In case of success, this function never returns.
		 * @throw std::system_error if an error occurs
		 */
		void start()
		{
			osStatus_t sta = osKernelStart();
			if (sta != osOK)
				throw std::system_error(cmsis::error_code(sta), "osKernelStart");
		}
	}

	namespace core
	{
		uint32_t clock_frequency()
		{
			if (!SystemCoreClock)
				throw std::system_error(cmsis::error_code(osError), "SystemCoreClock");

			return SystemCoreClock;
		}
	}

	/**
	 * Lock / unlock Dispatching.
	 * @throw std::system_error if an error occurs
	 */
	dispatch::dispatch() : m_previous_lock_state(osError)
	{
	}

	void dispatch::lock()
	{
		m_previous_lock_state = osKernelLock();
		if (m_previous_lock_state < 0)
			throw std::system_error(cmsis::error_code(m_previous_lock_state), "osKernelLock");
	}

	void dispatch::unlock()
	{
		if (m_previous_lock_state < 0)
			throw std::system_error(cmsis::error_code(m_previous_lock_state), "Bad kernel previous state");

		m_previous_lock_state = osKernelRestoreLock(m_previous_lock_state);
		if (m_previous_lock_state < 0)
			throw std::system_error(cmsis::error_code(m_previous_lock_state), "osKernelRestoreLock");
	}

	bool dispatch::locked()
	{
		return (osKernelGetState() == osKernelLocked);
	}
}
