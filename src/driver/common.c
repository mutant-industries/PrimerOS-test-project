// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/common.h>
#include <msp430.h>
#include <stdint.h>


// -------------------------------------------------------------------------------------

__resource Vector_handle_t vector;

// -------------------------------------------------------------------------------------

__resource Timer_config_t timer_config;
__resource Timer_driver_t timer_driver_1;
__resource Timer_driver_t timer_driver_2;

__resource Timer_channel_handle_t main_handle;
__resource Timer_channel_handle_t shared_handle_1;
__resource Timer_channel_handle_t shared_handle_2;
__resource Timer_channel_handle_t overflow_handle;
__resource Timer_channel_handle_t dummy_handle;

// -------------------------------------------------------------------------------------

__resource IO_port_driver_t IO_port_driver_1;
__resource IO_port_driver_t IO_port_driver_2;
__resource IO_port_driver_t IO_port_driver_3;
__resource IO_port_driver_t IO_port_driver_4;

__resource IO_pin_handle_t IO_pin_handle_1;
__resource IO_pin_handle_t IO_pin_handle_2;
__resource IO_pin_handle_t IO_pin_handle_3;
__resource IO_pin_handle_t IO_pin_handle_4;
__resource IO_pin_handle_t IO_pin_handle_5;
__resource IO_pin_handle_t IO_pin_handle_6;
__resource IO_pin_handle_t IO_pin_handle_7;
__resource IO_pin_handle_t IO_pin_handle_8;

// -------------------------------------------------------------------------------------

#define __TEST_TIMER_NO__           1


void default_timer_init(Timer_driver_t *driver, Timer_channel_handle_t *main_handle, Timer_channel_handle_t *shared_handle,
                                Timer_channel_handle_t *overflow_handle) {

    Timer_config_t config;

    default_clock_setup(0, DCOFSEL_0, DIVM__1);

    uint8_t main_vector_no = TIMER_A_VECTOR(__TEST_TIMER_NO__, TIMER_VECTOR_MAIN);
    uint8_t shared_vector_no = TIMER_A_VECTOR(__TEST_TIMER_NO__, TIMER_VECTOR_SHARED);

    config.mode = MC__CONTINOUS;
    config.clock_source = TASSEL__SMCLK;
    config.clock_source_divider = ID__1;
    config.clock_source_divider_expansion = TAIDEX__1;

    timer_driver_register(driver, &config, TIMER_A_BASE(__TEST_TIMER_NO__), main_vector_no, shared_vector_no, 2);

    if (main_handle) {
        timer_driver_channel_register(driver, main_handle, MAIN, NULL);
    }

    if (shared_handle) {
        timer_driver_channel_register(driver, shared_handle, SHARED, NULL);
    }

    if (overflow_handle) {
        timer_driver_channel_register(driver, overflow_handle, OVERFLOW, NULL);
    }
}
