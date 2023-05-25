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

void send_switched_temp_error(time_t time_command_sent, int64_t error_number)
{
	uint64_t send_error = 0;
	char valuemetadataid_errorpos[37] = "25c804d8-8c26-4b20-b429-0a8fce8949dc";
	char base64_string_error_number[20] = "";
	char string_error_timestamp [50] = "";
	char provide_value_message[500] = "";		// provide values output message

	bintob64(&base64_string_error_number[0],&error_number, sizeof(int64_t));

	do{
		send_error = (uint64_t) get_time();
	}while(send_error >= time_command_sent +30);

	send_error = (uint64_t) get_time() * 1000000;
	sprintf(string_error_timestamp, "%.0f", (double)send_error);
	sprintf(provide_value_message, "{\"TransactionNr\": %d,\"Operation\": \"ProvideValue\",\"ValueMetadataId\": \"%s\",\"Timestamp\": %s,\"Value\": \"%s\"}\r\n" , 666, valuemetadataid_errorpos, string_error_timestamp, base64_string_error_number);

  //++++++++++++++++++++++++++++++++++++++++++++++++++++
  // output via uart
  uart_output(&UART_OEM, provide_value_message);

  delay_ms(100);
}

int main(void)
{
  DAVE_STATUS_t status;
  time_t last_command_sent = 0;
  bool disable_provide_values;
  char valuemetadataid_temperature[37] = "";	// ValueMetaDataId 1
  char valuemetadataid_pressure[37] = "";		// ValueMetaDataId 2


  char base64_string_temperature[20] = "";      			// pointer to base64 string
  char base64_string_pressure[20] = "";
  char * provide_value_message;		// provide values output message
  char get_config_message[50] = "";
  char get_time_message[50] = "";
  char get_status_message[50] = "";
  uint64_t oem_unix_timestamp = 0;
  uint64_t temp_received_timestamp = 0;
  char string_unix_timestamp [50] = "";
//  bool send_temperature_next = true;
  // The OEM is working with a nanosecond timestamp the variable time_t would only work for seconds
  uint64_t received_unix_timestamp;
  XMC_RTC_TIME_t received_timestamp_in_seconds;
  int oem_info_gathering_status = 0;
  uint8_t oem_connection_status = 0;
  struct tm t;
  bool switched_temp_value = false;
  bool switched_pres_value = false;
  int64_t switch_error_number = 0;

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
  disable_provide_values = false; 		// true for a connection to the OEM via USB

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // init usb and uart interface
//  init_usb_Connection();
  init_uart_connection();

  if(!disable_provide_values)
  {
	  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	  // init i2c Interface
	  i2c_init();


  	  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  	  // init dps310
  	  dps310_init();
  }
  delay_ms(30000);
  while(1U)
  {
//	  if(USBD_VCOM_IsEnumDone() != 0)
//	  {
//		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
//		  // get usb input
//		  wait_for_input();

		  if(!new_uart_input_message && last_command_sent + 20 < get_time() && !disable_provide_values)
		  {

			  switch(oem_info_gathering_status)
			  {

		  	  	  case 0:
		  	  		  build_get_configuration(get_config_message,"1");
		  	  		  uart_output(&UART_OEM,get_config_message);
		  	  		  get_config_transactionnr = 1;
		  	  		  last_command_sent = get_time();
		  	  		  break;

		  	  	  case 1:
		  	  		  build_get_status(get_status_message, "2");
		  	  		  uart_output(&UART_OEM, get_status_message);
		  	  		  last_command_sent = get_time();
		  	  		  break;

		  	  	  case 2:
		  	  		  build_get_time(get_time_message, "3");
		  	  		  uart_output(&UART_OEM, get_time_message);
		  	  		  last_command_sent = get_time();
		  	  		  break;

		  	  	  case 3:
		  	  		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  	  		  // get temperature and pressure from dps310
		  	  		  dps310_get_cont_results();

		  	  		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  	  		  // get temperature from max31855
		  	  		  //get_max31855_temp(&SPI_MASTER_0, &DIGITAL_IO_0, &max31855_temp_external, &max31855_temp_internal);

		  	  		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  	  		  // increase transaction number
		  	  		  increase_transaction_nr();

		  	  		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  	  		  // build base64 strings from values and build send string
		  	  		  provide_value_message = calloc(500,sizeof(char));
		  	  		  memset(&base64_string_temperature[0], 0x0, 20);

					  bintob64(&base64_string_temperature[0],&dps310_status.temp_meas, sizeof(float));
					  if(dps310_status.temp_meas >= 40)
					  {
						  switched_temp_value = true;
					  }
					  //base64_string_temperature = base64_encode(&max31855_temp_external, sizeof(float), &base64_length);

					  // The OEM works with nanosec and the RTC works with seconds
					  oem_unix_timestamp = (uint64_t) get_time() * 1000000;
					  sprintf(string_unix_timestamp, "%.0f", (double)oem_unix_timestamp);

					  build_provide_value(provide_value_message,transaction_nr_string,valuemetadataid_temperature,base64_string_temperature,string_unix_timestamp);

		  	  		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  	  		  // output via usb
//		  	  		  USBD_VCOM_SendData((int8_t*) provide_value_message, strlen(provide_value_message));
//		  	  		  CDC_Device_USBTask(&USBD_VCOM_cdc_interface);

		  	  		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  	  		  // output via uart
		  	  		  uart_output(&UART_OEM,provide_value_message);

		  	  		  delay_ms(100);
//		  	  		  free(base64_string);
		  	  		  free(provide_value_message);

		  	  		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  	  		  // save timestamp
		  	  		  last_command_sent = get_time();
		  	  		  if(switched_temp_value == true)
		  	  		  {
		  	  			switch_error_number = 11;
		  	  			send_switched_temp_error(last_command_sent, switch_error_number);
		  	  		  }
		  	  	      oem_info_gathering_status = 4;
		  	  		  break;

		  	  	  case 4:

		  	  		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  	  		  // increase transaction number
		  	  		  increase_transaction_nr();

		  	  		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  	  		  // build base64 strings from values and build send string
		  	  		  provide_value_message = calloc(500,sizeof(char));
		  	  		  memset(&base64_string_pressure[0], 0x0, 20);

	  	  			  bintob64(&base64_string_pressure[0],&dps310_status.pres_meas, sizeof(float));
	  	  			  if(dps310_status.pres_meas <= 40)
	  	  			  {
	  	  				  switched_pres_value = true;
	  	  			  }

	  	  			  // The OEM works with nanosec and the RTC works with seconds
	  	  			  oem_unix_timestamp = (uint64_t) get_time() * 1000000;
	  	  			  sprintf(string_unix_timestamp, "%.0f", (double)oem_unix_timestamp);

	  	  			  build_provide_value(provide_value_message,transaction_nr_string,valuemetadataid_pressure,base64_string_pressure,string_unix_timestamp);

		  	  		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  	  		  // output via uart
		  	  		  uart_output(&UART_OEM,provide_value_message);

		  	  		  delay_ms(100);
//		  	  		  free(base64_string);
		  	  		  free(provide_value_message);

		  	  		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  	  		  // save timestamp
		  	  		  last_command_sent = get_time();
		  	  		  if(switched_pres_value == true)
		  	  		  {
		  	  			switch_error_number = -11;
		  	  			send_switched_temp_error(last_command_sent, switch_error_number);
		  	  		  }
	  	  			  oem_info_gathering_status = 3;
		  	  		  break;

		  	  	  default:
		  	  		  break;
			  }

		  }
		  else if (new_uart_input_message && !disable_provide_values )
		  {
//			  //++++++++++++++++++++++++++++++++++++++++++++++++++++
//			  // output on usb
//			  USBD_VCOM_SendData((int8_t*) uart_buffer, strlen(uart_buffer));
//			  CDC_Device_USBTask(&USBD_VCOM_cdc_interface);

			  switch(oem_info_gathering_status)
			  {

		  	  	  case 0:
		  	  		  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		  	  		  // parse oem response and save configuration
		  	  		  parse_oem_response_save_configuration(uart_buffer, strlen(uart_buffer));

		  	  		  get_valueMetaDataId("Temperature", valuemetadataid_temperature);
		  	  		  get_valueMetaDataId("Pressure", valuemetadataid_pressure);

		  	  		  if(strcmp(valuemetadataid_temperature,"") != 0 && strcmp(valuemetadataid_pressure,"") != 0)
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
		  	  		  received_unix_timestamp = parse_get_time(uart_buffer, strlen(uart_buffer));
		  	  		  if(received_unix_timestamp != 0)
		  	  		  {
		  	  			  temp_received_timestamp = received_unix_timestamp / 1000000;
						  t = *localtime((time_t*)&temp_received_timestamp);
						  received_timestamp_in_seconds.year = t.tm_year + 1900;
						  received_timestamp_in_seconds.month = t.tm_mon + 1;
						  received_timestamp_in_seconds.days = t.tm_mday;
						  received_timestamp_in_seconds.hours = t.tm_hour;
						  received_timestamp_in_seconds.minutes = t.tm_min;
						  received_timestamp_in_seconds.seconds = t.tm_sec;
						  received_timestamp_in_seconds.daysofweek = t.tm_wday;
		  	  			  RTC_SetTime(&received_timestamp_in_seconds);
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
//	  }
  }
}
