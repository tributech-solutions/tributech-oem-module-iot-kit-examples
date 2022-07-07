/*
 * led_blink.h
 *
 *  Created on: 21 Feb 2022
 *      Author: AEAscher
 */

extern const struct device *led;

int init_led();

void toggle_led();

void start_timer();