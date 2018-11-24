// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/process/lifecycle.h>
#include <kernel.h>
#include <process.h>


#define __INIT_PROCESS_PRIORITY__   1
#define __TEST_PROCESS_PRIORITY__   0
#define __TEST_STACK_SIZE__         0x6E

// -------------------------------------------------------------------------------------

static int16_t test_process_entry_point(void *);
static void test_process_exit_hook(int16_t);

static Timer_driver_t driver;
static Timer_channel_handle_us_convertible_t context_switch_handle;

static Process_control_block_t init_process, test_process;
static Process_create_config_t test_process_config;

static uint8_t test_stack[__TEST_STACK_SIZE__];

#ifdef _DATA_MODEL_LARGE_
static void *test_param = (void *) 0x1A5A5;
#else
static void *test_param = (void *) 0xB4B4;
#endif

static int16_t test_return_value = 0x4B4B;

static volatile bool test_process_executed;
static volatile bool exit_hook_executed;

// -------------------------------------------------------------------------------------

static void init() {
    zerofill(&test_process_config);

    test_process_config.priority = __TEST_PROCESS_PRIORITY__;
    test_process_config.stack_addr_low = test_stack;
    test_process_config.stack_size = __TEST_STACK_SIZE__;
    test_process_config.entry_point = test_process_entry_point;
    test_process_config.argument = test_param;

    // --- create and schedule another process with lower priority than init process ---
    process_create(&test_process, &test_process_config);
}

void test_kernel_process_lifecycle() {

    int16_t return_value;

    WDT_disable();

    default_timer_init(&driver, (Timer_channel_handle_t *) &context_switch_handle, NULL, NULL);

    test_process_executed = exit_hook_executed = false;

    // --- create and schedule process from current context ---
    if (kernel_start(&init_process, __INIT_PROCESS_PRIORITY__, init, false, (Context_switch_handle_t *) &context_switch_handle, NULL)) {
        test_fail();
    }

    // --- test execution order and exit code passing ---
    if (test_process_executed || ! test_process.alive) {
        test_fail();
    }

    return_value = process_wait(&test_process);

    if ( ! test_process_executed || test_process.alive) {
        test_fail();
    }

    if ( ! exit_hook_executed) {
        test_fail();
    }

    if (return_value != test_return_value) {
        test_fail();
    }

    // --- test wait on disposed process ---
    return_value = process_wait(&test_process);

    if (return_value != KERNEL_DISPOSED_RESOURCE_ACCESS) {
        test_fail();
    }

    // --- test kernel halt ---
    process_kill(&init_process);

    if (init_process.alive) {
        test_fail();
    }

    dispose(&driver);
}

static int16_t test_process_entry_point(void *arg) {

    if (arg != test_param) {
        test_fail();
    }

    test_process_executed = true;

    process_current_set_exit_hook(test_process_exit_hook);

    return test_return_value;
}

static void test_process_exit_hook(int16_t exit_code) {

    if (exit_code != test_return_value) {
        test_fail();
    }

    exit_hook_executed = true;
}
