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

uint32_t transaction_nr_dec;	// transaction number decimal
char transaction_nr_string[7];	// transaction number string

uint32_t _msCounter;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// String To LowerCase and UpperCase
char * to_lower_case(char * text);
char * to_upper_case(char * text);

void delay_ms(uint32_t ms);


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Get Time
time_t get_time(void);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// increase transaction number
void increase_transaction_nr(void);

#endif
