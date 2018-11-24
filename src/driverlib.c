// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driverlib.h>
#include <test/common.h>
#include <msp430.h>

// -------------------------------------------------------------------------------------

#define __TEST_TIMER_NO__           1


void default_timer_init(Timer_driver_t *driver, Timer_channel_handle_t *main_handle,
                                Timer_channel_handle_t *shared_handle, Timer_channel_handle_t *overflow_handle) {

    Timer_config_t config;

    default_clock_setup();

    uint8_t main_vector_no = TIMER_A_VECTOR(__TEST_TIMER_NO__, TIMER_VECTOR_MAIN);
    uint8_t shared_vector_no = TIMER_A_VECTOR(__TEST_TIMER_NO__, TIMER_VECTOR_SHARED);

    config.mode = MC__CONTINOUS;
    config.clock_source = TASSEL__SMCLK;
    config.clock_source_divider = ID__1;
    config.clock_source_divider_expansion = TAIDEX__1;

    timer_driver_register(driver, &config, TIMER_A_BASE(__TEST_TIMER_NO__), main_vector_no, shared_vector_no, 2);

    if (main_handle) {
        driver->channel_handle_register(driver, main_handle, MAIN, NULL);
    }

    if (shared_handle) {
        driver->channel_handle_register(driver, shared_handle, SHARED, NULL);
    }

    if (overflow_handle) {
        driver->channel_handle_register(driver, overflow_handle, OVERFLOW, NULL);
    }
}
