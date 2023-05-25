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

char valuemetadataid_temperature[37] = "";
char valuemetadataid_pressure[37] = "";
int oem_info_gathering_status = 0;

char base64_string_temperature[20] = "";
char base64_string_pressure[20] = "";
char * provide_value_message;		// provide values output message
char get_config_message[50] = "";
char get_time_message [50] = "";
char get_status_message [50] = "";
char string_unix_timestamp [50] = "";
uint8_t oem_connection_status = 0;
uint8_t return_code = 0;
// the timestamp uses nanosec and time_t would use seconds
uint64_t received_unix_timestamp;

void main(void)
{
    LOG_INF("Hello DPS368\n");
    uart_init();
    dps368_init();
    workqueue_init_and_start();
    start_unix_timer();

    LOG_INF("dev %p name %s\n", data.i2c_master, data.i2c_master->name);
    // This k_sleep is needed to allow the Tributech OEM module to initialize 
    k_sleep(K_MSEC(30000));

    while (1) 
    {
        if(!new_uart_message)
        {
            switch (oem_info_gathering_status)
            {
                // get configuration from OEM
                case 0:
                    build_get_configuration(get_config_message,"1");
                    get_config_transactionnr = 1;

                    LOG_INF("%s", get_config_message);
                    uart_tx(uart, get_config_message, strlen(get_config_message) + 1, 500);  
                    break;
                // get connection status from OEM
                case 1:
                    build_get_status(get_status_message, "2");

                    LOG_INF("%s", get_status_message);
                    uart_tx(uart, get_status_message, strlen(get_status_message) + 1, 500);
                    break;
                // get timestamp from OEM
                case 2:
                    build_get_time(get_time_message, "3");

                    LOG_INF("%s", get_time_message);
                    uart_tx(uart, get_time_message, strlen(get_time_message) +1, 500);
                    break;
                // send temperature value to the OEM
                case 3:
                    if(tmp_available)
                    {
                        //++++++++++++++++++++++++++++++++++++++++++++++++++++
                        // increase transaction number
                        increase_transaction_nr();

                        //++++++++++++++++++++++++++++++++++++++++++++++++++++
		    	        // build base64 strings from values and build send string
		    	        provide_value_message = k_calloc(500,sizeof(char));
                        memset(base64_string_temperature, 0x0, sizeof(char)*20);

                        if(tmp_available)
                        {       
		    		        bintob64(base64_string_temperature,&data.tmp_val, sizeof(float));

                            sprintf(string_unix_timestamp, "%llu" , unix_timestamp);

		    		        build_provide_value(provide_value_message,transaction_nr_string,valuemetadataid_temperature,base64_string_temperature, string_unix_timestamp);

                            tmp_available = false;
                        }

                        //++++++++++++++++++++++++++++++++++++++++++++++++++++
		    	        // output via uart
                        LOG_INF("%s", provide_value_message);
                        uart_tx(uart, provide_value_message, strlen(provide_value_message) + 1, 500);

                        k_sleep(K_SECONDS(59));
		    	        k_free(provide_value_message);

                        oem_info_gathering_status = 4;
                    }
                    break;
                case 4:
                    if(psr_available)
                    {
                        //++++++++++++++++++++++++++++++++++++++++++++++++++++
                        // increase transaction number
                        increase_transaction_nr();

                        //++++++++++++++++++++++++++++++++++++++++++++++++++++
		    	        // build base64 strings from values and build send string
		    	        provide_value_message = k_calloc(500,sizeof(char));
                        memset(base64_string_pressure, 0x0, sizeof(char)*20);

                        if (psr_available)
                        {
		    		        bintob64(base64_string_pressure,&data.psr_val, sizeof(float));

                            sprintf(string_unix_timestamp, "%llu" , unix_timestamp);

		    		        build_provide_value(provide_value_message,transaction_nr_string,valuemetadataid_pressure,base64_string_pressure, string_unix_timestamp);

                            psr_available = false;
                        }

                        //++++++++++++++++++++++++++++++++++++++++++++++++++++
		    	        // output via uart
                        LOG_INF("%s", provide_value_message);
                        uart_tx(uart, provide_value_message, strlen(provide_value_message) + 1, 500);

                        k_sleep(K_SECONDS(59));
		    	        k_free(provide_value_message);

                        oem_info_gathering_status = 3;
                    }
                    break;

                default:
                    break;
            }
        }
        if(new_uart_message)
        {
            switch (oem_info_gathering_status)
            {
                // get configuration from OEM
                case 0:
                    return_code = parse_oem_response_save_configuration(uart_receive_buf, uart_receive_len);
            
                    if(return_code)
                    {
                        get_valueMetaDataId("Temperature", valuemetadataid_temperature);
                        get_valueMetaDataId("Pressure", valuemetadataid_pressure);

                        oem_info_gathering_status = 1;
                        new_uart_message = false;
                        memset(uart_receive_buf, 0x0, 4096);
                        uart_reinit();
                    }
                    else
                    {
                        new_uart_message = false;
                        memset(uart_receive_buf, 0x0, 4096);
                        uart_reinit();
                    }
                    break;
                // get connection status from OEM
                case 1:
                    oem_connection_status = parse_get_status(uart_receive_buf, uart_receive_len);
                    if(oem_connection_status == 2)
                    {
                        LOG_INF("The OEM has successfuly connected to the node.");
                        oem_info_gathering_status = 2;
                        new_uart_message = false;
                        memset(uart_receive_buf, 0x0, 4096);
                        uart_reinit();
                    }
                    else
                    {
                        LOG_INF("OEM not connected to node. Waiting...\n");
                        new_uart_message = false;
                        memset(uart_receive_buf, 0x0, 4096);
                        uart_reinit();
                        k_sleep(K_MSEC(2000));
                    }
                    break;
                // get timestamp from OEM
                case 2:
                    received_unix_timestamp = parse_get_time(uart_receive_buf, uart_receive_len);
                    LOG_INF("Timestamp set to : %llu", received_unix_timestamp);
                    if(received_unix_timestamp != 0)
                    {
                        unix_timestamp = received_unix_timestamp;
                        oem_info_gathering_status = 3;
                        new_uart_message = false;
                        memset(uart_receive_buf, 0x0, 4096);
                        uart_reinit();
                    }
                    else
                    {
                        new_uart_message = false;
                        memset(uart_receive_buf, 0x0, 4096);
                        uart_reinit();
                    }
                    break;
                default:
                    LOG_INF("%s", uart_receive_buf);
                    new_uart_message = false;
                    memset(uart_receive_buf, 0x0, 4096);
                    uart_reinit();
                    break;
            }
        }
    k_sleep(K_MSEC(1000));
    }
}
