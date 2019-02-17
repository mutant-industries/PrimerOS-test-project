// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/common.h>
#include <kernel.h>


// -------------------------------------------------------------------------------------

__resource Timing_handle_t context_switch_handle;
__resource Timing_handle_t timing_handle_1;
__resource Timing_handle_t timing_handle_2;

// -------------------------------------------------------------------------------------

__resource Process_control_block_t init_process;
__resource Process_control_block_t process_1;
__resource Process_control_block_t process_2;

// -------------------------------------------------------------------------------------

__resource uint8_t test_stack_1[__TEST_STACK_SIZE__];
__resource uint8_t test_stack_2[__TEST_STACK_SIZE__];

// -------------------------------------------------------------------------------------

__resource Action_t action_1;
__resource Action_t action_2;
__resource Action_t action_3;
__resource Action_t action_4;
__resource Action_t action_5;

__resource Action_queue_t action_queue_1;
__resource Action_queue_t action_queue_2;
__resource Action_queue_t action_queue_3;
__resource Action_queue_t action_queue_4;

// -------------------------------------------------------------------------------------

__resource Schedule_config_t init_process_schedule_config;
__resource Schedule_config_t process_1_schedule_config;
__resource Schedule_config_t process_2_schedule_config;

// -------------------------------------------------------------------------------------

__resource Semaphore_t semaphore_1;
__resource Semaphore_t semaphore_2;

__resource Mutex_t mutex_1;
__resource Mutex_t mutex_2;

// -------------------------------------------------------------------------------------

__resource Event_t event_1;
__resource Event_t event_2;

__resource Subscription_t subscription_1;
__resource Subscription_t subscription_2;

// -------------------------------------------------------------------------------------

__resource Timed_signal_t timed_signal_1;
__resource Timed_signal_t timed_signal_2;

__resource Schedule_config_t timed_signal_1_schedule_config;
__resource Schedule_config_t timed_signal_2_schedule_config;

// -------------------------------------------------------------------------------------

void default_kernel_start(priority_t init_process_priority, void (*sys_init)(void), bool wakeup) {

    default_timer_init(&timer_driver_1, _timer_channel_handle_(&context_switch_handle), _timer_channel_handle_(&timing_handle_1), NULL);

    timing_handle_1.usecs_to_ticks = NULL;
    timing_handle_1.ticks_to_usecs = NULL;
    timing_handle_1.timer_counter_bit_width = 16;

    assert( ! kernel_start(&init_process, init_process_priority, sys_init,
                         wakeup, (Context_switch_handle_t *) &context_switch_handle, &timing_handle_1));
}
