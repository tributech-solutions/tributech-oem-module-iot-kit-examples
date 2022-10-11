/*
 * Copyright (c) 2019 Infineon Technologies AG
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>

#include <drivers/sensor.h>
#include <stdlib.h>
#include <stdio.h>
#include <uart_communication.h>
#include <dps368.h>
#include <scheduling.h>
#include <parse_and_send.h>
#include <logging/log.h>
#include <tributech.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

bool stream_valuemetadata_ids_received = false;
char get_configuration[60] = "{\"TransactionNr\": 1, \"Operation\": \"GetConfiguration\"}\r\n";
uint16_t get_configuration_len = 60;
uint8_t return_code = 0;

void main(void)
{
    LOG_INF("Hello DPS368\n");
    uart_init();
    dps368_init();
    workqueue_init_and_start();
    transaction_nr = 1;

    LOG_INF("dev %p name %s\n", data.i2c_master, data.i2c_master->name);
    // This k_sleep is needed to allow the Tributech OEM module to initialize 
    k_sleep(K_MSEC(30000));

    while (1) 
    {
        if(!new_uart_message && !stream_valuemetadata_ids_received)
        {
            uart_tx(uart, get_configuration, get_configuration_len+1, 500);
            get_config_transactionnr = 1;
        }
        if(stream_valuemetadata_ids_received)
        {
            send_tmp_and_psr_value_to_oem();
        }
        if(new_uart_message == true)
        {
            return_code = parse_oem_response_save_configuration(uart_receive_buf, uart_receive_len);
            
            if(return_code)
            {
                get_valueMetaDataId("Temperature", valuemetadataid_tmp);
                get_valueMetaDataId("Pressure", valuemetadataid_psr);

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
