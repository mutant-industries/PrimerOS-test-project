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
        src/driver/common.c
        src/driver/wdt/disabled.c
        src/driver/wdt/watchdog.c
        src/driver/wdt/interval.c
        src/driver/vector/trigger.c
        src/driver/timer/dispose.c
        src/driver/timer/multiple.c
        src/driver/stack/pointer.c
        src/driver/stack/deferred.c
        src/kernel/common.c
        src/kernel/dispose/chain.c
        src/kernel/dispose/resource.c
        src/kernel/process/lifecycle.c
        src/kernel/queue/sorting.c
        src/kernel/queue/fifo.c
        src/kernel/queue/priority.c
        src/kernel/sync/barrier.c
        src/kernel/sync/inversion.c
        src/kernel/event/trigger.c
        src/kernel/event/inheritance.c)

target_include_directories(PrimerOS-test-project
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
        PRIVATE src)

target_compile_definitions(MSP430-driverlib PUBLIC ${DEFINITIONS})
target_compile_definitions(PrimerOS PUBLIC ${DEFINITIONS})
target_compile_definitions(PrimerOS-test-project PUBLIC ${DEFINITIONS})

target_link_libraries(PrimerOS-test-project MSP430-driverlib PrimerOS)
