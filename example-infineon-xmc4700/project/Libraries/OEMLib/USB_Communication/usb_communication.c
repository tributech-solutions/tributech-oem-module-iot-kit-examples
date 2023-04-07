///*
// * usb_communication.c
// *
// *  Created on: 21 Jun 2022
// *      Author: AndreasAscher
// */
//
//
//#include "usb_communication.h"
//#include "timer_definition.h"
//#include "uart_communication.h"
//
//uint32_t 	usb_recv_index = 0;
//uint32_t 	last_usb_recv_ms;
//
////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//// init usb connection
//int16_t init_usb_Connection()
//{
//	int16_t return_status = USBD_VCOM_STATUS_FAILURE;
//
//	return_status = USBD_VCOM_Connect();			// Connect to USB
//
//	/* Wait until enumeration is finished */
//	while (!USBD_VCOM_IsEnumDone());
//	usb_received_message = calloc(USB_RECEIVE_BUFFER_SIZE, sizeof(char));	// Alloc usb read buffer
//
//	return return_status;
//}
//
////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//// wait for usb input
//int16_t wait_for_input()
//{
//	uint16_t bytes = 0;
//	int16_t return_status = 0;
//    bytes = USBD_VCOM_BytesReceived();
//    if (bytes > 0)
//    {
//
//		//+++++++++++++++++++++++++++++++++++++++++++
//		// receive bytes
//    	return_status = (int16_t)USBD_VCOM_ReceiveData((int8_t*) &usb_received_message[usb_recv_index],bytes);
//    	CDC_Device_USBTask(&USBD_VCOM_cdc_interface);
//
//		last_usb_recv_ms = _msCounter;
//		usb_recv_index = usb_recv_index + bytes;
//		if(return_status != USBD_STATUS_SUCCESS)
//		{
//			return return_status;
//		}
//    }
//    else if (bytes == 0 && usb_recv_index > 0 && (usb_received_message[usb_recv_index-1] == 0x0D || usb_received_message[usb_recv_index-1] == 0x0A))
//    {
//    	//+++++++++++++++++++++++++++++++++++++++++++
//		// return sended characters -> echo
////    	return_status = (int16_t)USBD_VCOM_SendData((int8_t*)usb_received_message,strlen(usb_received_message));
////		CDC_Device_USBTask(&USBD_VCOM_cdc_interface);
//    	if(usb_received_message[usb_recv_index-1] == 0x0A && usb_received_message[usb_recv_index-2] != 0x0D)
//    	{
//    		usb_received_message[usb_recv_index-1] = 0x0D;
//    		usb_received_message[usb_recv_index] = 0x0A;
//    	}
//
//		//+++++++++++++++++++++++++++++++++++++++++++
//		// Parse command
//		usb_recv_index = strlen(usb_received_message);
////		return_status = ssm_parse_command(usb_received_message,usb_recv_index);
//		uart_output(&UART_OEM, usb_received_message);
//
//
//		memset(usb_received_message,0x0,USB_RECEIVE_BUFFER_SIZE);
//		usb_recv_index = 0;
//		if(return_status != 0)
//		{
//			return return_status;
//		}
//    }
//    return return_status;
//}
