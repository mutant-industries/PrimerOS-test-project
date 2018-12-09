// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/common.h>
#include <kernel.h>


// -------------------------------------------------------------------------------------

Timer_channel_handle_us_convertible_t context_switch_handle;
Timer_channel_handle_us_convertible_t timing_handle;

// -------------------------------------------------------------------------------------

Process_control_block_t init_process;
Process_control_block_t process_1;
Process_control_block_t process_2;

// -------------------------------------------------------------------------------------

uint8_t test_stack_1[__TEST_STACK_SIZE__];
uint8_t test_stack_2[__TEST_STACK_SIZE__];

// -------------------------------------------------------------------------------------

Action_t action_1;
Action_t action_2;
Action_t action_3;
Action_t action_4;
Action_t action_5;

Action_queue_t action_queue_1;

// -------------------------------------------------------------------------------------

void default_kernel_start(priority_t init_process_priority, void (*sys_init)(void), bool wakeup) {

    default_timer_init(&timer_driver, _timer_channel_handle_(&context_switch_handle), _timer_channel_handle_(&timing_handle), NULL);

    assert( ! kernel_start(&init_process, init_process_priority, sys_init,
                         wakeup, (Context_switch_handle_t *) &context_switch_handle, &timing_handle));
}
