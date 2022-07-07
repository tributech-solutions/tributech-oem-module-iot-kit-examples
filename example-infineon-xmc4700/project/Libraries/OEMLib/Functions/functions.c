/*
 * functions.c
 *
 *  Created on: 17 Aug 2020
 *      Author: DanielHackl
 */

#include <functions.h>

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
// increase transaction number
void increase_transaction_nr(void)
{
	transaction_nr_dec++;
	if (transaction_nr_dec >= 1000000)
	{
		transaction_nr_dec = 0;
	}
	memset(transaction_nr_string,0x0,7);
	sprintf(transaction_nr_string, "%"PRIu32"",transaction_nr_dec);
}

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



