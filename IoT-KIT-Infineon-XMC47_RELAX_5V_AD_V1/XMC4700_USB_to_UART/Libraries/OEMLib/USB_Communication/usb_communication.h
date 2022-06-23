/*
 * usb_communication.h
 *
 *  Created on: 21 Jun 2022
 *      Author: AndreasAscher
 */

#ifndef LIBRARIES_OEMLIB_USB_COMMUNICATION_USB_COMMUNICATION_H_
#define LIBRARIES_OEMLIB_USB_COMMUNICATION_USB_COMMUNICATION_H_

#include <DAVE.h>

#define USB_RECEIVE_BUFFER_SIZE 8192

char * usb_received_message;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// init usb connection
int16_t init_usb_Connection();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// wait for usb input
int16_t wait_for_input();

#endif /* LIBRARIES_OEMLIB_USB_COMMUNICATION_USB_COMMUNICATION_H_ */
