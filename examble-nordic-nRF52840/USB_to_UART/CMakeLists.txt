#
# Copyright (c) 2020 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(low_power_uart)

FILE(GLOB app_sources src/*.c)

# include uart_communication.h
zephyr_include_directories(src/UartCommunication)

#include led_blink.h
zephyr_include_directories(src/LEDblink)

# include uart_communication.c
target_sources(app PRIVATE src/UartCommunication/uart_communication.c)

#include led_blink
target_sources(app PRIVATE src/LEDblink/led_blink.c)

# NORDIC SDK APP START
target_sources(app PRIVATE
	${app_sources}
)
# NORDIC SDK APP END
