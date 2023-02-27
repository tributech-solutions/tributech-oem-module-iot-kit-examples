/*
 * functions.c
 *
 *  Created on: 17 Aug 2020
 *      Author: DanielHackl
 */

#include <functions.h>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Get Time
time_t get_time(void)
{
	time_t Time_Sec;

	RTC_Time(&Time_Sec);

	return Time_Sec;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Delay ms and us
void delay_ms(uint32_t ms)
{
	uint32_t now = _msCounter;

	while (now + ms > _msCounter)
					__NOP();
}



