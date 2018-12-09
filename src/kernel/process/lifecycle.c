// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/process/lifecycle.h>
#include <kernel.h>
#include <process.h>


#define __INIT_PROCESS_PRIORITY__   1
#define __TEST_PROCESS_PRIORITY__   0

// -------------------------------------------------------------------------------------

static signal_t test_process_entry_point(signal_t);
static void test_process_exit_action_handler(Action_t *, signal_t);

#ifdef _DATA_MODEL_LARGE_
static void *test_param = (void *) 0x1A5A5;
#else
static void *test_param = (void *) 0xB4B4;
#endif

static signal_t test_return_value = signal(0x4B4B);

static volatile bool test_process_executed;
static volatile bool exit_action_executed;

// -------------------------------------------------------------------------------------

static void init() {
    zerofill(&process_1.create_config);

    process_1.create_config.priority = __TEST_PROCESS_PRIORITY__;
    process_1.create_config.stack_addr_low = (data_pointer_register_t) test_stack_1;
    process_1.create_config.stack_size = __TEST_STACK_SIZE__;
    process_1.create_config.entry_point = (process_entry_point_t) test_process_entry_point;
    process_1.create_config.arg_1 = test_param;

    // --- create another process with lower priority than init process ---
    process_create(&process_1);
    // ... and schedule it
    process_schedule(&process_1, 0);
}

void test_kernel_process_lifecycle() {

    signal_t return_value;

    WDT_disable();

    test_process_executed = exit_action_executed = false;

    // --- create and schedule process from current context ---
    default_kernel_start(__INIT_PROCESS_PRIORITY__, init, false);

#ifndef __ASYNC_API_DISABLE__
    // --- test async action execution on process exit ---
    action_register(&action_1, NULL, test_process_exit_action_handler, NULL, NULL, NULL);

    assert(process_register_dispose_action(&process_1, &action_1));
#endif

    // --- test execution order and exit code passing ---
    assert( ! test_process_executed && process_1.alive);

    return_value = process_wait(&process_1, NULL);

    assert(test_process_executed && ! process_1.alive);

#ifndef __ASYNC_API_DISABLE__
    assert(exit_action_executed);
#endif

    assert(return_value == test_return_value);

    // --- test wait on disposed process ---
    return_value = process_wait(&process_1, NULL);

    assert(return_value == PROCESS_SIGNAL_EXIT);

    // --- test kernel halt ---
    process_kill(&init_process);

    assert_not(init_process.alive);

    dispose(&timer_driver);
}

static signal_t test_process_entry_point(signal_t arg) {

    assert(arg == test_param);

    test_process_executed = true;

    return test_return_value;
}

static void test_process_exit_action_handler(Action_t *_this, signal_t exit_code) {

    assert(exit_code == test_return_value);

    exit_action_executed = true;
}
