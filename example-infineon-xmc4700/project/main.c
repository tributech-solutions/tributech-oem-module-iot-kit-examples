/*
 * main.c
 *
 *  Created on: 2022 Jun 21 11:10:10
 *  Author: AndreasAscher
 */




#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include <functions.h>
#include <tributech_oem_api.h>
#include "usb_communication.h"
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






int main(void)
{
  DAVE_STATUS_t status;
  time_t last_command_sent;
  bool disable_provide_values = false;
  bool mode_sending_with_break = false;
  bool stream_ids_received = false;
  char valuemetadataid_float[37] = "";
  char valuemetadataid_double[37] = "";
  char valuemetadataid_int32[37] = "";
  char valuemetadataid_int64[37] = "";
  char valuemetadataid_step[37] = "";
  char valuemetadataid_pmin[37] = "";
  char valuemetadataid_pmax[37] = "";

  char *base64_string;      			// pointer to base64 string
  char * provide_values_message;		// provide values output message
  char get_config_message[50] = "";
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
  disable_provide_values = false; 		// true for linking
  mode_sending_with_break = false;		// true for sending with break

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // init usb and uart interface
  init_usb_Connection();
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
  while(1U)
  {
	  if(USBD_VCOM_IsEnumDone() != 0)
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
		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  // get usb input
		  wait_for_input();

		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  // if no configuration received -> send getConfiguration command
		  if (!configuration_received && !disable_provide_values)// && !disable_provide_values)
		  {
			  if (last_command_sent + 10 < get_time())
			  {
				  build_get_configuration(get_config_message,"1");
				  uart_output(&UART_OEM,get_config_message);
				  get_config_transactionnr = 1;
				  last_command_sent = get_time();
			  }
		  }
		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  // configuration received -> parse ids
		  else if (configuration_received && !stream_ids_received && !disable_provide_values)// && !disable_provide_values)
		  {
			  get_valueMetaDataId("Float Stream", valuemetadataid_float);
			  get_valueMetaDataId("Double Stream", valuemetadataid_double);
			  get_valueMetaDataId("INT32 Stream", valuemetadataid_int32);
			  get_valueMetaDataId("INT64 Stream", valuemetadataid_int64);
			  get_valueMetaDataId("STEP Stream", valuemetadataid_step);
			  get_valueMetaDataId("PMIN Stream", valuemetadataid_pmin);
			  get_valueMetaDataId("PMAX Stream", valuemetadataid_pmax);

			  if(strcmp(valuemetadataid_float,"") != 0 || strcmp(valuemetadataid_double,"") != 0)
			  {
				  stream_ids_received = true;
			  }
		  }
		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  // ids received -> publish values
		  else if(last_command_sent + 10 < get_time() && stream_ids_received && !disable_provide_values && !sending_break)
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
					build_provide_values(provide_values_message,transaction_nr_string,valuemetadataid_float,base64_string,"0");
				}
				else if (send_index == 2 && strcmp(valuemetadataid_double,"") != 0)
				{
					double_value = double_value + 0.5;

					bintob64(base64_string,&double_value, sizeof(double));
					build_provide_values(provide_values_message,transaction_nr_string,valuemetadataid_double,base64_string,"0");
				}

				else if (send_index == 3 && strcmp(valuemetadataid_int32,"") != 0)
				{
					int32_value++;

					bintob64(base64_string,&int32_value, sizeof(int32_t));
					build_provide_values(provide_values_message,transaction_nr_string,valuemetadataid_int32,base64_string,"0");
				}

				else if (send_index == 4 && strcmp(valuemetadataid_int64,"") != 0)
				{
					int64_value++;

					bintob64(base64_string,&int64_value, sizeof(int64_t));
					build_provide_values(provide_values_message,transaction_nr_string,valuemetadataid_int64,base64_string,"0");
				}
				else if (send_index == 5 && strcmp(valuemetadataid_step,"") != 0)
				{
					float_step_value = float_step_value + 0.5;
					bintob64(base64_string,&float_step_value, sizeof(float));
					build_provide_values(provide_values_message,transaction_nr_string,valuemetadataid_step,base64_string,"0");
				}
				else if (send_index == 6 && strcmp(valuemetadataid_pmin,"") != 0)
				{
					float_pmin_value = float_pmin_value + 0.5;
					bintob64(base64_string,&float_pmin_value, sizeof(float));
					build_provide_values(provide_values_message,transaction_nr_string,valuemetadataid_pmin,base64_string,"0");
				}
				else if (send_index == 7 && strcmp(valuemetadataid_pmax,"") != 0)
				{
					float_pmax_value = float_pmax_value + 0.5;
					bintob64(base64_string,&float_pmax_value, sizeof(float));
					build_provide_values(provide_values_message,transaction_nr_string,valuemetadataid_pmax,base64_string,"0");
				}


				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				// output via usb
				USBD_VCOM_SendData((int8_t*) provide_values_message, strlen(provide_values_message));
				CDC_Device_USBTask(&USBD_VCOM_cdc_interface);

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
	  }

	  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	  // output on usb
	  if (new_usb_output_message == true)
	  {
		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  // output on usb
		  USBD_VCOM_SendData((int8_t*) uart_buffer, strlen(uart_buffer));
		  CDC_Device_USBTask(&USBD_VCOM_cdc_interface);
		  new_usb_output_message = false;

		  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		  // parse oem response and save configuration
		  parse_oem_response_save_configuration(uart_buffer, strlen(uart_buffer));

		  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		  // reset uart receive buffer
		  memset(uart_buffer,0x0,UART_RECEIVE_BUFFER_SIZE);
		  uart_read_index = 0;
	  }
  }
}
