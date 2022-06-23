/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <uart_communication.h>
#include <led_blink.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(app);

void main(void)
{
    printk("Sensor Test Start!\n");
    
    init_led();

    start_timer();
   
    usb_2_uart_init();

    while(1)
    {

    }
}
