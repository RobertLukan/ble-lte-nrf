#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nrf_cloud_mqtt_multi_service)
zephyr_compile_definitions(PROJECT_NAME=${PROJECT_NAME})

# NORDIC SDK APP START
target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/application.c)
target_sources(app PRIVATE src/connection.c)
target_sources(app PRIVATE src/temperature.c)
target_sources(app PRIVATE src/fota_support.c)
target_sources(app PRIVATE src/led_control.c)
target_sources(app PRIVATE src/location_tracking.c)
target_sources(app PRIVATE src/aggregator.c)
target_sources(app PRIVATE src/ble.c)
# NORDIC SDK APP END


include(../../../../nrf/cmake/boilerplate.cmake NO_POLICY_SCOPE)
zephyr_include_directories(src)
