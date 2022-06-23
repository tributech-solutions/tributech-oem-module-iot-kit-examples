/*
 * uart_communication.h
 *
 *  Created on: 21 Jun 2022
 *      Author: AndreasAscher
 */

#ifndef LIBRARIES_OEMLIB_UART_COMMUNICATION_UART_COMMUNICATION_H_
#define LIBRARIES_OEMLIB_UART_COMMUNICATION_UART_COMMUNICATION_H_

#include <DAVE.h>

#define UART_OUTPUT_BUFFER_SIZE 8192
#define UART_RECEIVE_BUFFER_SIZE  8192
#define UART_RECEIVE_CHARACTER_BUFFER_SIZE  50
char *uart_output_buffer;		// buffer for uart_output function

uint8_t uart_read_index;
uint8_t *uart_characters;
char *uart_buffer;
uint32_t uart_last_receive;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init UART Values
void init_uart_connection(void);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Uart Output
UART_STATUS_t uart_output(const UART_t *const handle,const char *string1);

#endif /* LIBRARIES_OEMLIB_UART_COMMUNICATION_UART_COMMUNICATION_H_ */
