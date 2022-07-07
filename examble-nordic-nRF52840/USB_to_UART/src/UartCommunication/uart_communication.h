/*
 * uart_communication.h
 *
 *  Created on: 21 Feb 2022
 *      Author: AEAscher
 */

#include <stdio.h>
#include <string.h>
#include <device.h>
#include <drivers/uart.h>
#include <zephyr.h>
#include <sys/ring_buffer.h>

#include <usb/usb_device.h>
#include <logging/log.h>

void usb_2_uart_init();