/*
 * main.c
 *
 *  Created on: 2022 Jun 21 11:10:10
 *  Author: AndreasAscher
 */




#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include <functions.h>
#include "usb_communication.h"
#include "uart_communication.h"
#include "dps310_ctrl.h"
#include "i2c_master_ctrl.h"
#include "base64.h"
#include "tributech.h"

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
  bool disable_provide_values;
  bool stream_ids_received;
  char valuemetadataid_temperature[37] = "";	// ValueMetaDataId 1
  char valuemetadataid_pressure[37] = "";		// ValueMetaDataId 2

  char *base64_string_temperature;      // pointer to base64 string
  char *base64_string_pressure;         // pointer to base64 string
  uint16_t base64_length;				// base64 length
  char * provide_values_message;		// provide values output message



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
  disable_provide_values = true; 		// true for linking

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // init usb and uart interface
  init_usb_Connection();
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

  while(1U)
  {

	  if(USBD_VCOM_IsEnumDone() != 0)
	  {
		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  // get usb input
		  wait_for_input();

		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  // if no configuration received -> send getConfiguration command
		  if (!configuration_received && !disable_provide_values)// && !disable_provide_values)
		  {
			  if (last_command_sent + 10 < get_time())
			  {
				  uart_output(&UART_OEM,"{\"TransactionNr\": 1, \"Operation\": \"GetConfiguration\"}\r\n");
				  get_config_transactionnr = 1;
				  last_command_sent = get_time();
			  }
		  }
		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  // configuration received -> parse ids
		  else if (configuration_received && !stream_ids_received && !disable_provide_values)// && !disable_provide_values)
		  {
			  get_valueMetaDataId("Temperature", valuemetadataid_temperature);
			  get_valueMetaDataId("Pressure", valuemetadataid_pressure);

			  if(strcmp(valuemetadataid_temperature,"") != 0 && strcmp(valuemetadataid_pressure,"") != 0)
			  {
				  stream_ids_received = true;
			  }
		  }
		  //++++++++++++++++++++++++++++++++++++++++++++++++++++
		  // ids received -> publish values
		  else if(last_command_sent + 10 < get_time() && stream_ids_received && !disable_provide_values)
		  {
				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				// get  temperature and pressure
				dps310_get_cont_results();

				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				// increase transaction number
				increase_transaction_nr();

				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				// build base64 strings from values
				base64_string_temperature = base64_encode(&dps310_status.temp_meas, sizeof(float), &base64_length);
				base64_string_pressure = base64_encode(&dps310_status.pres_meas, sizeof(float), &base64_length);

				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				// build provide values message
				provide_values_message = calloc(500,sizeof(char));
				sprintf(provide_values_message, "{\"TransactionNr\": %s,\"Operation\": \"ProvideValues\",\"ValueMetadataId\": \"%s\",\"Values\": [{\"Timestamp\": 0,\"Value\": \"%s\"}],\"ValueMetadataId\": \"%s\",\"Values\": [{\"Timestamp\": 0,\"Value\": \"%s\"}]}\r\n" , transaction_nr_string, valuemetadataid_temperature, base64_string_temperature, valuemetadataid_pressure, base64_string_pressure);

				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				// output via usb
				USBD_VCOM_SendData((int8_t*) provide_values_message, strlen(provide_values_message));
				CDC_Device_USBTask(&USBD_VCOM_cdc_interface);

				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				// output via uart
				uart_output(&UART_OEM,provide_values_message);

				delay_ms(100);
				free(base64_string_temperature);
				free(base64_string_pressure);
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
