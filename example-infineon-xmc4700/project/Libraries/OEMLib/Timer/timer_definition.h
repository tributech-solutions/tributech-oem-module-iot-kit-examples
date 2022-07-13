/*
 * timer_definition.h
 *
 *  Created on: 21 Jun 2022
 *      Author: AndreasAscher
 */

#ifndef LIBRARIES_OEMLIB_TIMER_TIMER_DEFINITION_H_
#define LIBRARIES_OEMLIB_TIMER_TIMER_DEFINITION_H_
#include <DAVE.h>

volatile uint32_t _msCounter;
volatile uint32_t _usCounter;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// System Ticker
void SysTick_Handler();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Check Timeout  -> return true if timeout reached
int check_timeout(uint32_t counter, uint32_t var, uint32_t timeout);

#endif /* LIBRARIES_OEMLIB_TIMER_TIMER_DEFINITION_H_ */
