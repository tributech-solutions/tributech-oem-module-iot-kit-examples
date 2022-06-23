/*
 * uart_communication.c
 *
 *  Created on: 21 Jun 2022
 *      Author: AndreasAscher
 */

#include "uart_communication.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init UART Values
void init_uart_connection(void)
{
	uart_characters = (uint8_t*)calloc(UART_RECEIVE_CHARACTER_BUFFER_SIZE, sizeof(uint8_t));
	uart_buffer = (char*)calloc(UART_RECEIVE_BUFFER_SIZE, sizeof(char));

	uart_output_buffer = (char*)calloc(UART_OUTPUT_BUFFER_SIZE, sizeof(char));

	UART_Receive(&UART_OEM, uart_characters,1);	// Start receiving
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Uart Output
UART_STATUS_t uart_output(const UART_t *const handle,const char *string1)
{
	memset(uart_output_buffer,0x0,UART_OUTPUT_BUFFER_SIZE);

	sprintf(uart_output_buffer, string1);

	while(UART_IsTxBusy(handle))
	{
	}

	return UART_Transmit(handle, (uint8_t*)uart_output_buffer,strlen(uart_output_buffer));
}

void interrupt_uart_oem_receive ()
{
	int i;
	USBD_VCOM_STATUS_t return_status = 0;

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Receive Byte
	UART_Receive(&UART_OEM, uart_characters,1);
	uart_buffer[uart_read_index] = uart_characters[0];	// Save it in command buffer


	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// If first char is CR or LF then delete it
	if (uart_read_index == 0 && (uart_buffer[0] == '\r' || uart_characters[0] == '\n'))
	{
		uart_read_index = 0;
	}
	else
	{
		//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// Save the last 50 chars
		for(i=UART_RECEIVE_CHARACTER_BUFFER_SIZE;i>=1;i--)
		{
			uart_characters[i] = uart_characters[i-1];
		}

		//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// increment index
		uart_read_index++;

		//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// if last char is LF
		if (uart_buffer[uart_read_index-2] == '\r' && uart_buffer[uart_read_index-1] == '\n')
		{
			return_status = USBD_VCOM_SendData((int8_t*) uart_buffer, uart_read_index);
			CDC_Device_USBTask(&USBD_VCOM_cdc_interface);

			//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			// Reset command buffer and index
			memset(uart_buffer,0x0,UART_RECEIVE_BUFFER_SIZE);
			uart_read_index = 0;
		}
	}
}
