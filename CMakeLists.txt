#
# Copyright (c) 2018 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.13.1)

#add custom board
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mqtt-simple)

#Fix <ff.h> not found
zephyr_include_directories( $ENV{ZEPHYR_BASE}/../modules/fs/fatfs/include)        #Add this line

#disable uart logging of boot sequence
set(mcuboot_CONF_FILE 
    prj.conf 
    mcuboot_config.conf
    )

#add local sources
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE src)

add_subdirectory(src/drivers)
add_subdirectory(src/devices)
add_subdirectory(src/ugui)
add_subdirectory(src/buffers)
add_subdirectory(src/tasks)
add_subdirectory(src/time)
add_subdirectory(src/mqtt)
add_subdirectory(src/local_log)
add_subdirectory(src/messages)

# NORDIC SDK APP START
target_sources(app PRIVATE src/main.c)
# NORDIC SDK APP END
