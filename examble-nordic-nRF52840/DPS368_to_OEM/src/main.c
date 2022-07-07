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

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

void main(void)
{
    LOG_INF("Hello DPS368\n");
    uart_init();
    dps368_init();
    workqueue_init_and_start();
    transaction_nr = 1;

    LOG_INF("dev %p name %s\n", data.i2c_master, data.i2c_master->name);

    while (1) 
    {
        send_tmp_and_psr_value_to_oem();
        k_sleep(K_MSEC(1000));
    }
}
