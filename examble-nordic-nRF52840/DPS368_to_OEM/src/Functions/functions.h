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
#include <zephyr.h>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// String To LowerCase and UpperCase
char * to_lower_case(char * text);
char * to_upper_case(char * text);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// minimum of two uint32_t values
uint32_t min2_uint32_t (uint32_t value1, uint32_t value2);

#endif
