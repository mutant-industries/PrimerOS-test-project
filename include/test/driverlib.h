/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  MSP430 test framework driverlib support functions
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _TEST_DRIVERLIB_H_
#define _TEST_DRIVERLIB_H_

#include <stddef.h>
#include <stdint.h>
#include <driver/vector.h>
#include <driver/timer.h>


void default_timer_init(Timer_driver_t *driver, Timer_channel_handle_t *main_handle,
                                Timer_channel_handle_t *shared_handle, Timer_channel_handle_t *overflow_handle);

#endif /* _TEST_DRIVERLIB_H_ */
