/*
 * main.c
 *
 *  Created on: 2022 Jun 21 11:10:10
 *  Author: AndreasAscher
 */

#include <zephyr.h>
#include <device.h>

#include <drivers/sensor.h>
#include <stdlib.h>
#include <stdio.h>
#include <uart_communication.h>
#include <dps368.h>
#include <scheduling.h>
#include <Base64.h>
#include <logging/log.h>
#include <tributech_oem_api.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

bool stream_valuemetadata_ids_received = false;
char valuemetadataid_temperature[37] = "";
char valuemetadataid_pressure[37] = "";

char *base64_string;      			// pointer to base64 string
char * provide_values_message;		// provide values output message
char get_config_message[50] = "";
uint8_t return_code = 0;

void main(void)
{
    LOG_INF("Hello DPS368\n");
    uart_init();
    dps368_init();
    workqueue_init_and_start();

    LOG_INF("dev %p name %s\n", data.i2c_master, data.i2c_master->name);
    // This k_sleep is needed to allow the Tributech OEM module to initialize 
    k_sleep(K_MSEC(30000));

    while (1) 
    {
        if(!new_uart_message && !stream_valuemetadata_ids_received)
        {
            build_get_configuration(get_config_message,"1");
            get_config_transactionnr = 1;

            LOG_INF("%s", get_config_message);
            uart_tx(uart, get_config_message, strlen(get_config_message) + 1, 500);  
        }
        if(stream_valuemetadata_ids_received && (tmp_available || psr_available))
        {
            //++++++++++++++++++++++++++++++++++++++++++++++++++++
            // increase transaction number
            increase_transaction_nr();

            //++++++++++++++++++++++++++++++++++++++++++++++++++++
			// build base64 strings from values and build send string
			provide_values_message = calloc(500,sizeof(char));
            base64_string = calloc(20,sizeof(char));

            if(tmp_available)
            {
				bintob64(base64_string,&data.tmp_val, sizeof(float));

				build_provide_values(provide_values_message,transaction_nr_string,valuemetadataid_temperature,base64_string,"0");
        
                tmp_available = false;
            }
            else if (psr_available)
            {
				bintob64(base64_string,&data.psr_val, sizeof(float));

				build_provide_values(provide_values_message,transaction_nr_string,valuemetadataid_pressure,base64_string,"0");

                psr_available = false;
            }

            //++++++++++++++++++++++++++++++++++++++++++++++++++++
			// output via uart
            LOG_INF("%s", provide_values_message);
            uart_tx(uart, provide_values_message, strlen(provide_values_message) + 1, 500);

            k_sleep(K_MSEC(100));

            free(base64_string);
			free(provide_values_message);

        }
        if(new_uart_message)
        {
            return_code = parse_oem_response_save_configuration(uart_receive_buf, uart_receive_len);
            
            if(return_code)
            {
                get_valueMetaDataId("Temperature", valuemetadataid_temperature);
                get_valueMetaDataId("Pressure", valuemetadataid_pressure);

                new_uart_message = false;
                stream_valuemetadata_ids_received = true;
            }
            else
            {
                memset(uart_receive_buf, 0x0, 4096);
                uart_reinit();
                new_uart_message = false;
                stream_valuemetadata_ids_received = false;
            }
        }
        k_sleep(K_MSEC(1000));
    }
}
