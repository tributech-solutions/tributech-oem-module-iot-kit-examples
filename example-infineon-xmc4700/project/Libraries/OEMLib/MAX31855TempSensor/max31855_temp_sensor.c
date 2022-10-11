/* * max31855_temp_sensor.c
 *
 *  Created on: 11 Oct 2022
 *      Author: DanielHackl
 */

#include "max31855_temp_sensor.h"

float max31855_temp_external;
float max31855_temp_internal;

int get_max31855_temp(SPI_MASTER_t *const handle, const DIGITAL_IO_t *const cs, float* external_temp, float* internal_temp)
{
	uint8_t ReadData[10];
	uint16_t temp_external;
	uint16_t temp_internal;

	DIGITAL_IO_SetOutputLow(cs);	// CS signal low

	if(SPI_MASTER_Receive(handle, ReadData, 4))
	{
	  while(handle->runtime->rx_busy)
	  {
	  }
	}

	DIGITAL_IO_SetOutputHigh(cs);	// CS signal high

	//+++++++++++++++++++++++++++++++++++
	// transform external 14-Bit temp value
	memset(&temp_external,0x0,2);


	memcpy(&temp_external,ReadData,1);
	temp_external = temp_external << 8;
	memcpy(&temp_external,ReadData+1,1);

	*external_temp = temp_external >> 2;
	*external_temp = *external_temp * 0.25;

	//+++++++++++++++++++++++++++++++++++
	// transform internal 12-Bit temp value
	memset(&temp_internal,0x0,2);
	memcpy(&temp_internal,ReadData+2,1);
	temp_internal = temp_internal << 8;
	memcpy(&temp_internal,ReadData+3,1);

	*internal_temp = temp_internal >> 4;
	*internal_temp = *internal_temp * 0.0625;

	return 0;
}
