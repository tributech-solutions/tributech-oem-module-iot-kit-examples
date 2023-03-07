# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(dps310)
FILE(GLOB app_sources src/*.c)

# include uart_communication
zephyr_include_directories(src/UartCommunication)
target_sources(app PRIVATE src/UartCommunication/uart_communication.c)

# include dps368 scheduling
zephyr_include_directories(src/Scheduling)
target_sources(app PRIVATE src/Scheduling/scheduling.c)

# include dps368
zephyr_include_directories(src/DPS368)
target_sources(app PRIVATE src/DPS368/dps368.c)

#include Tributech lib
zephyr_include_directories(src/TributechLib)
target_sources(app PRIVATE src/TributechLib/tributech_oem_api.c)

#include jsmn json parser
zephyr_include_directories(src/JSMNParser)
target_sources(app PRIVATE src/JSMNParser/jsmn.c)

# include BASE64
zephyr_include_directories(src/BASE64)
target_sources(app PRIVATE src/BASE64/BASE64.c)

target_sources(app PRIVATE ${app_sources})
