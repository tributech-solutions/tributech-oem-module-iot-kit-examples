/*
 * main.c
 *
 *  Created on: 2022 Jun 21 11:10:10
 *  Author: AndreasAscher
 */




#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include <functions.h>
#include <tributech_oem_api.h>
//#include "usb_communication.h"
#include "uart_communication.h"
#include "dps310_ctrl.h"
#include "i2c_master_ctrl.h"
#include "base64.h"
#include "max31855_temp_sensor.h"

/**
 * @brief main() - Application entry point
 *
 * <b>Details of function</b><br>
 * This routine is the application entry point. It is invoked by the device startup code. It is responsible for
 * invoking the APP initialization dispatcher routine - DAVE_Init() and hosting the place-holder for user application
 * code.
 */

struct tm t;

int main(void)
{
  DAVE_STATUS_t status;
  time_t last_command_sent = 0;
  bool mode_sending_with_break = false;
  char valuemetadataid_float[37] = "";
  char valuemetadataid_double[37] = "";
  char valuemetadataid_int32[37] = "";
  char valuemetadataid_int64[37] = "";
  char valuemetadataid_step[37] = "";
  char valuemetadataid_pmin[37] = "";
  char valuemetadataid_pmax[37] = "";

  char *base64_string;      			// pointer to base64 string
  char * provide_values_message;		// provide values output message
  char get_config_message[100] = "";
  char get_status_message[50] = "";
  char get_time_message[50] = "";
  uint64_t oem_unix_timestamp = 0;
  uint64_t temp_received_timestamp = 0;
  XMC_RTC_TIME_t received_timestamp_in_seconds;
  char string_unix_timestamp[50] = "";
  float float_value = 0;
  float float_step_value = 0;
  float float_pmin_value = 0;
  float float_pmax_value = 0;
  double double_value = 0;
  int32_t int32_value = 0;
  int64_t int64_value = 0;
  int send_index = 0;
  uint32_t break_counter = 0;
  bool sending_break = false;
  time_t last_time;
  int oem_info_gathering_status = 0;
  uint8_t oem_connection_status = 0;

  status = DAVE_Init();           /* Initialization of DAVE APPs  */

  if (status != DAVE_STATUS_SUCCESS)
  {
    /* Placeholder for error handler code. The while loop below can be replaced with an user error handler. */
    XMC_DEBUG("DAVE APPs initialization failed\n");

    while(1U)
    {

    }
  }

  SysTick_Config(SystemCoreClock / 1000);	// Systemtick 1 ms

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // The following boolean disables the provide values function.
  // This means that the sensor values from the DPS368 are not used anymore and the user is able to send data to the OEm via the  COM port.
  mode_sending_with_break = false;		// true for sending with break

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // init usb and uart interface
//  init_usb_Connection();
  init_uart_connection();
  /*
  if(!disable_provide_values)
  {
	  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	  // init i2c Interface
	  i2c_init();


  	  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  	  // init dps310
  	  dps310_init();
  }
  */
  delay_ms(30000);
  while(1U)
  {
	  if (mode_sending_with_break)
	  {
		  if (last_time != get_time())	// every second
		  {
			  break_counter = break_counter + 1;
			  last_time = get_time();
		  }

		  if (break_counter > 3600)		// break after 1h
		  {
			  break_counter = 0;
			  sending_break = !sending_break;
		  }
	  }

	  if(!new_uart_input_message)
	  {
		  switch(oem_info_gathering_status)
		  {
			  case 0:
				  if (last_command_sent + 10 < get_time())
				  {
					  build_get_configuration(get_config_message,"1");
					  uart_output(&UART_OEM,get_config_message);
					  get_config_transactionnr = 1;
					  last_command_sent = get_time();
				  }
				  break;
			  case 1:
				  if (last_command_sent + 10 < get_time())
				  {
					  build_get_status(get_status_message, "2");

					  uart_output(&UART_OEM, get_status_message);

					  last_command_sent = get_time();
				  }
				  break;
			  case 2:
				  if (last_command_sent + 10 < get_time())
				  {
					  build_get_time(get_time_message, "3");

					  uart_output(&UART_OEM, get_time_message);

					  last_command_sent = get_time();
				  }
				  break;
			  case 3:
				  if (last_command_sent + 10 < get_time() && !sending_break)
				  {
						//++++++++++++++++++++++++++++++++++++++++++++++++++++
						// increase transaction number
						increase_transaction_nr();

						//++++++++++++++++++++++++++++++++++++++++++++++++++++
						// build base64 strings from values and build send string
						base64_string = calloc(20,sizeof(char));
						provide_values_message = calloc(500,sizeof(char));

						send_index++;
						if (send_index > 7)
						{
							send_index = 1;
						}

						if (send_index == 1 && strcmp(valuemetadataid_float,"") != 0)
						{
							float_value = float_value + 0.5;
							bintob64(base64_string,&float_value, sizeof(float));

							oem_unix_timestamp = (uint64_t) get_time() * 1000000;
							sprintf(string_unix_timestamp, "%.0f", (double)oem_unix_timestamp);

							build_provide_value(provide_values_message,transaction_nr_string,valuemetadataid_float,base64_string, string_unix_timestamp);
						}
						else if (send_index == 2 && strcmp(valuemetadataid_double,"") != 0)
						{
							double_value = double_value + 0.5;

							bintob64(base64_string,&double_value, sizeof(double));

							oem_unix_timestamp = (uint64_t) get_time() * 1000000;
							sprintf(string_unix_timestamp, "%.0f", (double)oem_unix_timestamp);

							build_provide_value(provide_values_message,transaction_nr_string,valuemetadataid_double,base64_string, string_unix_timestamp);
						}

						else if (send_index == 3 && strcmp(valuemetadataid_int32,"") != 0)
						{
							int32_value++;

							bintob64(base64_string,&int32_value, sizeof(int32_t));

							oem_unix_timestamp = (uint64_t) get_time() * 1000000;
							sprintf(string_unix_timestamp, "%.0f", (double)oem_unix_timestamp);

							build_provide_value(provide_values_message,transaction_nr_string,valuemetadataid_int32,base64_string, string_unix_timestamp);
						}

						else if (send_index == 4 && strcmp(valuemetadataid_int64,"") != 0)
						{
							int64_value++;

							bintob64(base64_string,&int64_value, sizeof(int64_t));

							oem_unix_timestamp = (uint64_t) get_time() * 1000000;
							sprintf(string_unix_timestamp, "%.0f", (double)oem_unix_timestamp);

							build_provide_value(provide_values_message,transaction_nr_string,valuemetadataid_int64,base64_string, string_unix_timestamp);
						}
						else if (send_index == 5 && strcmp(valuemetadataid_step,"") != 0)
						{
							float_step_value = float_step_value + 0.5;
							bintob64(base64_string,&float_step_value, sizeof(float));

							oem_unix_timestamp = (uint64_t) get_time() * 1000000;
							sprintf(string_unix_timestamp, "%.0f", (double)oem_unix_timestamp);

							build_provide_value(provide_values_message,transaction_nr_string,valuemetadataid_step,base64_string, string_unix_timestamp);
						}
						else if (send_index == 6 && strcmp(valuemetadataid_pmin,"") != 0)
						{
							float_pmin_value = float_pmin_value + 0.5;
							bintob64(base64_string,&float_pmin_value, sizeof(float));

							oem_unix_timestamp = (uint64_t) get_time() * 1000000;
							sprintf(string_unix_timestamp, "%.0f", (double)oem_unix_timestamp);

							build_provide_value(provide_values_message,transaction_nr_string,valuemetadataid_pmin,base64_string, string_unix_timestamp);
						}
						else if (send_index == 7 && strcmp(valuemetadataid_pmax,"") != 0)
						{
							float_pmax_value = float_pmax_value + 0.5;
							bintob64(base64_string,&float_pmax_value, sizeof(float));

							oem_unix_timestamp = (uint64_t) get_time() * 1000000;
							sprintf(string_unix_timestamp, "%.0f", (double)oem_unix_timestamp);

							build_provide_value(provide_values_message,transaction_nr_string,valuemetadataid_pmax,base64_string, string_unix_timestamp);
						}


//						//++++++++++++++++++++++++++++++++++++++++++++++++++++
//						// output via usb
//						USBD_VCOM_SendData((int8_t*) provide_values_message, strlen(provide_values_message));
//						CDC_Device_USBTask(&USBD_VCOM_cdc_interface);

						//++++++++++++++++++++++++++++++++++++++++++++++++++++
						// output via uart
						uart_output(&UART_OEM,provide_values_message);

						delay_ms(100);
						free(base64_string);
						free(provide_values_message);

						//++++++++++++++++++++++++++++++++++++++++++++++++++++
						// save timestamp
						last_command_sent = get_time();
				  }
				  break;
			  default:
				  break;
		  }
	  }
	  else
	  {
//		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
//		  // output on usb
//		  USBD_VCOM_SendData((int8_t*) uart_buffer, strlen(uart_buffer));
//		  CDC_Device_USBTask(&USBD_VCOM_cdc_interface);

		  switch(oem_info_gathering_status)
		  {
		  	  case 0:
				  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				  // parse oem response and save configuration
				  parse_oem_response_save_configuration(uart_buffer, strlen(uart_buffer));

				  get_valueMetaDataId("Float Stream", valuemetadataid_float);
				  get_valueMetaDataId("Double Stream", valuemetadataid_double);
				  get_valueMetaDataId("INT32 Stream", valuemetadataid_int32);
				  get_valueMetaDataId("INT64 Stream", valuemetadataid_int64);
				  get_valueMetaDataId("STEP Stream", valuemetadataid_step);
				  get_valueMetaDataId("PMIN Stream", valuemetadataid_pmin);
				  get_valueMetaDataId("PMAX Stream", valuemetadataid_pmax);

				  if(strcmp(valuemetadataid_float,"") != 0 || strcmp(valuemetadataid_double,"") != 0)
				  {
					  oem_info_gathering_status = 1;
				  }
		  		  break;
		  	  case 1:
		  		  oem_connection_status = parse_get_status(uart_buffer, strlen(uart_buffer));

		  		  if(oem_connection_status == 2)
		  		  {
		  			  oem_info_gathering_status = 2;
		  		  }
		  		  else
		  		  {
		  			  delay_ms(2000);
		  		  }
		  		  break;
		  	  case 2:
		  		  temp_received_timestamp = parse_get_time(uart_buffer, strlen(uart_buffer));
		  		  if(temp_received_timestamp != 0)
		  		  {
		  			  temp_received_timestamp = temp_received_timestamp / 1000000;
		  			  t = *localtime((time_t*)&temp_received_timestamp);
		  			  received_timestamp_in_seconds.year = t.tm_year + 1900;
		  			  received_timestamp_in_seconds.month = t.tm_mon + 1;
		  			  received_timestamp_in_seconds.days = t.tm_mday;
		  			  received_timestamp_in_seconds.hours = t.tm_hour;
		  			  received_timestamp_in_seconds.minutes = t.tm_min;
		  			  received_timestamp_in_seconds.seconds = t.tm_sec;
		  			  received_timestamp_in_seconds.daysofweek = t.tm_wday;

		  			  RTC_SetTime(&received_timestamp_in_seconds);
		  			  last_command_sent = get_time();
		  			  oem_info_gathering_status = 3;
		  		  }
		  		  break;
		  	  default:
		  		  break;
		  }

		  new_uart_input_message = false;

		  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		  // reset uart receive buffer
		  memset(uart_buffer,0x0,UART_RECEIVE_BUFFER_SIZE);
		  uart_read_index = 0;
	  }
  }
}
