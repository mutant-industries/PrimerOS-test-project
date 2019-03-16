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
#include <driver/IO.h>
#include <driver/DMA.h>

// -------------------------------------------------------------------------------------

extern Vector_handle_t vector;

// -------------------------------------------------------------------------------------

extern Timer_config_t timer_config;
extern Timer_driver_t timer_driver_1;
extern Timer_driver_t timer_driver_2;

extern Timer_channel_handle_t main_handle;
extern Timer_channel_handle_t shared_handle_1;
extern Timer_channel_handle_t shared_handle_2;
extern Timer_channel_handle_t overflow_handle;
extern Timer_channel_handle_t dummy_handle;

// -------------------------------------------------------------------------------------

extern IO_port_driver_t IO_port_driver_1;
extern IO_port_driver_t IO_port_driver_2;
extern IO_port_driver_t IO_port_driver_3;
extern IO_port_driver_t IO_port_driver_4;

extern IO_pin_handle_t IO_pin_handle_1;
extern IO_pin_handle_t IO_pin_handle_2;
extern IO_pin_handle_t IO_pin_handle_3;
extern IO_pin_handle_t IO_pin_handle_4;
extern IO_pin_handle_t IO_pin_handle_5;
extern IO_pin_handle_t IO_pin_handle_6;
extern IO_pin_handle_t IO_pin_handle_7;
extern IO_pin_handle_t IO_pin_handle_8;

// -------------------------------------------------------------------------------------

extern DMA_driver_t DMA_driver;
extern DMA_channel_handle_t DMA_handle_1;
extern DMA_channel_handle_t DMA_handle_2;
extern DMA_channel_handle_t DMA_handle_3;
extern DMA_channel_handle_t DMA_handle_4;
extern DMA_channel_handle_t DMA_handle_5;
extern DMA_channel_handle_t DMA_handle_6;

// -------------------------------------------------------------------------------------

void default_timer_init(Timer_driver_t *driver, Timer_channel_handle_t *main_handle, Timer_channel_handle_t *shared_handle,
                                Timer_channel_handle_t *overflow_handle);


#endif /* _TEST_DRIVER_COMMON_H_ */
