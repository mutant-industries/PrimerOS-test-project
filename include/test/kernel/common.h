/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  PrimerOS tests common data structures
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _TEST_KERNEL_COMMON_H_
#define _TEST_KERNEL_COMMON_H_

#include <test/common.h>
#include <test/driver/common.h>
#include <defs.h>
#include <action.h>
#include <action/queue.h>
#include <process.h>
#include <sync/semaphore.h>
#include <sync/mutex.h>
#include <time.h>

// -------------------------------------------------------------------------------------

#define __TEST_STACK_SIZE__         0x8E

// -------------------------------------------------------------------------------------

extern Timer_channel_handle_us_convertible_t context_switch_handle;
extern Timer_channel_handle_us_convertible_t timing_handle;

// -------------------------------------------------------------------------------------

extern Process_control_block_t init_process;
extern Process_control_block_t process_1;
extern Process_control_block_t process_2;

// -------------------------------------------------------------------------------------

extern uint8_t test_stack_1[__TEST_STACK_SIZE__];
extern uint8_t test_stack_2[__TEST_STACK_SIZE__];

// -------------------------------------------------------------------------------------

extern Action_t action_1;
extern Action_t action_2;
extern Action_t action_3;
extern Action_t action_4;
extern Action_t action_5;

extern Action_queue_t action_queue_1;

// -------------------------------------------------------------------------------------

extern Schedule_config_t init_process_schedule_config;
extern Schedule_config_t process_1_schedule_config;
extern Schedule_config_t process_2_schedule_config;

// -------------------------------------------------------------------------------------

extern Semaphore_t semaphore_1;
extern Semaphore_t semaphore_2;

extern Mutex_t mutex_1;
extern Mutex_t mutex_2;

// -------------------------------------------------------------------------------------

void default_kernel_start(priority_t init_process_priority, void (*sys_init)(void), bool wakeup);


#endif /* _TEST_KERNEL_COMMON_H_ */
