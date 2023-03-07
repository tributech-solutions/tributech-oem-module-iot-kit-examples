/*
 * functions.h
 *
 *  Created on: 17 Aug 2020
 *      Author: DanielHackl
 */

#ifndef LIBRARIES_TRIBUTECH_SSM_USB_HOST_LIBRARY_TRIBUTECH_LIB_H_
#define LIBRARIES_TRIBUTECH_SSM_USB_HOST_LIBRARY_TRIBUTECH_LIB_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <dave.h>


uint32_t _msCounter;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// delay ms
void delay_ms(uint32_t ms);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Get Time
time_t get_time(void);




#endif
