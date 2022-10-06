/*
 * uart_communication.h
 *
 *  Created on: 26 May 2022
 *      Author: A.Ascher
 */

#ifndef UART_COMMUNICATION_H_
#define UART_COMMUNICATION_H_

#include <zephyr.h>
#include <drivers/uart.h>

#define UART_RECEIVE_BUFFER_SIZE 4096
extern char uart_receive_buf[UART_RECEIVE_BUFFER_SIZE];
extern uint32_t uart_receive_index;
extern uint16_t uart_receive_len;
extern bool new_uart_message;
extern const struct device *uart;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// uart callback
void uart_callback(const struct device *dev, struct uart_event *evt, void *user_data);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// init uart
void uart_init(void);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// init uart
void uart_reinit(void);

#endif /* UART_COMMUNICATION_H_ */