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
#include <Base64.h>
#include <logging/log.h>
#include <tributech_oem_api.h>


LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

char valuemetadataid_temperature[37] = "a0a675dd-c0af-4cc4-a77c-f1f4dda384a8";
char valuemetadataid_pressure[37] = "041f0040-b27c-42b2-a1d0-ce9225ede4a5";
int oem_info_gathering_status = 0;

char base64_string_temperature[20] = "";
char base64_string_pressure[20] = "";
char provide_value_message_temperature[500] = "";;		// provide values output message
char provide_value_message_pressure[500] = "";
char get_config_message[50] = "";
char get_time_message [50] = "";
char get_status_message [50] = "";
uint8_t oem_connection_status = 0;
uint8_t return_code = 0;
// the timestamp uses nanosec and time_t would use seconds
char string_unix_timestamp[50] = "";
float temperature = 25;
float pressure = 95000;
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

    // LOG_INF("dev %p name %s\n", data.i2c_master, data.i2c_master->name);

    // Set Stage for static timer transmission
    //temperature base64
    bintob64(base64_string_temperature, &temperature, sizeof(float));
    //pressure base64
    bintob64(base64_string_pressure, &pressure, sizeof(float));
    sprintf(string_unix_timestamp, "%d", 0x0);

    increase_transaction_nr();

    //temperature command
    build_provide_value(provide_value_message_temperature, transaction_nr_string, valuemetadataid_temperature, base64_string_temperature, string_unix_timestamp);

    //pressure command
    build_provide_value(provide_value_message_pressure, transaction_nr_string, valuemetadataid_pressure, base64_string_pressure, string_unix_timestamp);

    // This k_sleep is needed to allow the Tributech OEM module to initialize 
    k_sleep(K_MSEC(60000));

    // Start dps368 function with a 2 minute periode
    k_timer_start(&dps368_timer, K_SECONDS(20), K_SECONDS(20));

    while (1) 
    {

    }
}

K_TIMER_DEFINE(dps368_timer, dps368_timer_handler, NULL);