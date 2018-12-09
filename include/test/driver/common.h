/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  MSP430 driverlib tests common data structures
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _TEST_DRIVER_COMMON_H_
#define _TEST_DRIVER_COMMON_H_

#include <test/common.h>
#include <driver/vector.h>
#include <driver/timer.h>

// -------------------------------------------------------------------------------------

extern Vector_handle_t vector;

// -------------------------------------------------------------------------------------

extern Timer_config_t timer_config;
extern Timer_driver_t timer_driver;

extern Timer_channel_handle_t main_handle;
extern Timer_channel_handle_t shared_handle_1;
extern Timer_channel_handle_t shared_handle_2;
extern Timer_channel_handle_t overflow_handle;
extern Timer_channel_handle_t dummy_handle;

// -------------------------------------------------------------------------------------

void default_timer_init(Timer_driver_t *driver, Timer_channel_handle_t *main_handle, Timer_channel_handle_t *shared_handle,
                                Timer_channel_handle_t *overflow_handle);


#endif /* _TEST_DRIVER_COMMON_H_ */
