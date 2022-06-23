/*
 * timer_definition.c
 *
 *  Created on: 21 Jun 2022
 *      Author: AndreasAscher
 */

#include "timer_definition.h"


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// System Ticker
void SysTick_Handler()
{
	_usCounter++;						// increase us counter
	_msCounter = _usCounter / 1000;		// get ms counter

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Check Timeout  -> return true if timeout reached
int check_timeout(uint32_t counter, uint32_t var, uint32_t timeout)
{
	if (counter > var + timeout || (counter < var && var > timeout))
	{
		return true;
	}
	else
	{
		return false;
	}
}
