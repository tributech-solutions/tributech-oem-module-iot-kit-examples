/*
 * main.c
 *
 *  Created on: 2022 Jun 21 11:10:10
 *  Author: AndreasAscher
 */

#include <zephyr.h>
#include <kernel.h>
#include <device.h>

#include <drivers/sensor.h>
#include <stdlib.h>
#include <stdio.h>
#include <uart_communication.h>
// #include <dps368.h>
#include <scheduling.h>
// #include <Base64.h>
#include <logging/log.h>
// #include <tributech_oem_api.h>


LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

// char valuemetadataid_temperature[37] = "09002d61-b0f4-4971-8034-500b1775eeab";
// char valuemetadataid_pressure[37] = "7d5a71d3-c384-4778-a66b-87f3c15a8795";
int oem_info_gathering_status = 0;

// char base64_string_temperature[20] = "";
// char base64_string_pressure[20] = "";
char provide_value_message_temperature[500] = "{\"TransactionNr\": 1, \"Operation\": \"ProvideValue\",\"ValueMetadataId\": \"09002d61-b0f4-4971-8034-500b1775eeab\", \"Timestamp\": 0, \"Value\": \"AADIQQ==\"}\r\n";
char provide_value_message_pressure[500] = "{\"TransactionNr\": 2, \"Operation\": \"ProvideValue\",\"ValueMetadataId\": \"7d5a71d3-c384-4778-a66b-87f3c15a8795\", \"Timestamp\": 0, \"Value\": \"AIy5Rw==\"}\r\n";
uint8_t return_code = 0;
uint64_t cycle_counter = 0;

// timer definitions
struct k_timer dps368_timer;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// Dps368 timer capture handler
static void dps368_timer_handler(struct k_timer *dummy)
{
    if((cycle_counter % 2) == 0)
    {
        uart_tx(uart, provide_value_message_temperature, strlen(provide_value_message_temperature) + 1, 500);
    }
    else
    {
        uart_tx(uart, provide_value_message_pressure, strlen(provide_value_message_pressure) + 1, 500);
    }
    cycle_counter++;
}

void main(void)
{
    LOG_INF("Hello DPS368\n");
    uart_init();

    // This k_sleep is needed to allow the Tributech OEM module to initialize 
    k_sleep(K_MSEC(60000));

    // Start dps368 function with a 2 minute periode
    k_timer_start(&dps368_timer, K_SECONDS(30), K_SECONDS(30));

    while (1) 
    {

    }
}

K_TIMER_DEFINE(dps368_timer, dps368_timer_handler, NULL);