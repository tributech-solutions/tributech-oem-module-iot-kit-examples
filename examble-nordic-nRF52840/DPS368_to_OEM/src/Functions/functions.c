/*
 * functions.c
 *
 *  Created on: 17 Aug 2020
 *      Author: DanielHackl
 */

#include <functions.h>
#include <string.h>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// String To UpperCase
char * to_upper_case(char * text)
{
	uint16_t i;

	i=0;
	while(i < strlen(text))
	{
		text[i] = (char) toupper((int)text[i]);
	    i++;
	}

	return text;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// String To LowerCase
char * to_lower_case(char * text)
{
	uint16_t i;

	i=0;
	while(i < strlen(text))
	{
		text[i] = (char) tolower((int)text[i]);
	    i++;
	}

	return text;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// minimum of two uint32_t values
uint32_t min2_uint32_t (uint32_t value1, uint32_t value2)
{
	if (value2 < value1)
	{
		return value2;
	}
	return value1;
}
