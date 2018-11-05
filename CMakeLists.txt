# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2018-2019 Mutant Industries ltd.
cmake_minimum_required(VERSION 3.2)

project(PrimerOS-test-project VERSION 0.1 LANGUAGES C)

if (NOT TARGET_MCU)
    set(TARGET_MCU MSP430FR5994)
endif()

set(DEFINITIONS
        __${TARGET_MCU}__
        __RESOURCE_MANAGEMENT_ENABLE__)

add_executable(PrimerOS-test-project
        src/runner.c
        src/common.c
        src/driver/wdt/disabled.c
        src/driver/wdt/watchdog.c
        src/driver/wdt/interval.c
        src/driver/disposable/chain.c
        src/driver/disposable/resource.c
        src/driver/vector/trigger.c
        src/driver/timer/dispose.c
        src/driver/timer/multiple.c)

target_include_directories(PrimerOS-test-project
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
        PRIVATE src)

target_compile_definitions(MSP430-driverlib PUBLIC ${DEFINITIONS})
target_compile_definitions(PrimerOS PUBLIC ${DEFINITIONS})
target_compile_definitions(PrimerOS-test-project PUBLIC ${DEFINITIONS})

target_link_libraries(PrimerOS-test-project MSP430-driverlib PrimerOS)
