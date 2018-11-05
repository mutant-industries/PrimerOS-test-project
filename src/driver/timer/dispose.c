// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/timer/dispose.h>
#include <driver/timer.h>
#include <process.h>

// -------------------------------------------------------------------------------------

#define __TEST_TIMER_NO__   1

static void test_interrupt_handler(Process_control_block_t *);
static dispose_function_t test_dispose_hook(Disposable_t *disposable);

static Timer_config_t config;
static Timer_driver_t driver;

static Timer_channel_handle_t main_handle;
static Timer_channel_handle_t shared_handle_1;
static Timer_channel_handle_t shared_handle_2;
static Timer_channel_handle_t overflow_handle;
static Timer_channel_handle_t dummy_handle;

static Process_control_block_t pcb;
static volatile uint8_t interrupt_handler_call_count;
static volatile bool dispose_hook_called;

// -------------------------------------------------------------------------------------

void test_driver_timer_dispose() {
    uint16_t main_vector_content_backup, shared_vector_content_backup, counter;

    running_process = &pcb;
    dispose_hook_called = false;
    interrupt_handler_call_count = 0;

    WDT_disable();
    default_clock_setup();

    uint8_t main_vector_no = TIMER_A_VECTOR(__TEST_TIMER_NO__, TIMER_VECTOR_MAIN);
    uint8_t shared_vector_no = TIMER_A_VECTOR(__TEST_TIMER_NO__, TIMER_VECTOR_SHARED);

    main_vector_content_backup = __vector(main_vector_no);
    shared_vector_content_backup = __vector(shared_vector_no);

    config.mode = MC__CONTINOUS;
    config.clock_source = TASSEL__SMCLK;
    config.clock_source_divider = ID__1;
    config.clock_source_divider_expansion = TAIDEX__1;

    __enable_interrupt();

    timer_driver_register(&driver, &config, TIMER_A_BASE(__TEST_TIMER_NO__), main_vector_no, shared_vector_no, 3);

    // --- main handle allocate ---
    if (driver.channel_handle_register(&driver, &main_handle, MAIN, (dispose_function_t) test_dispose_hook)) {
        test_fail();
    }

    if (main_handle.vector.register_handler(&main_handle.vector, (void (*)(void *)) test_interrupt_handler, &pcb) == NULL) {
        test_fail();
    }

    if (main_vector_content_backup == __vector(main_vector_no)) {
        test_fail();
    }

    // --- main handle trigger ---
    if (interrupt_handler_call_count) {
        test_fail();
    }

    if (main_handle.vector.trigger(&main_handle.vector)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 1) {
        test_fail();
    }

    if (main_handle.vector.trigger(&main_handle.vector)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 2) {
        test_fail();
    }

    // --- reset ---
    if (main_handle.reset(&main_handle)) {
        test_fail();
    }

    // --- timer counter ---
    if (main_handle.get_counter(&main_handle, &counter) || counter) {
        test_fail();
    }

    // --- main handle allocate when allocated already ---
    if (driver.channel_handle_register(&driver, &dummy_handle, MAIN,
               (dispose_function_t) test_dispose_hook) != TIMER_NO_HANDLE_AVAILABLE) {

        test_fail();
    }

    interrupt_handler_call_count = 0;

    // --- main handle dispose ---
    if (dispose_hook_called) {
        test_fail();
    }

    dispose(&main_handle);

    if ( ! dispose_hook_called) {
        test_fail();
    }

    if (main_vector_content_backup != __vector(main_vector_no)) {
        test_fail();
    }

    if ( ! main_handle.vector.trigger(&main_handle.vector) || ! main_handle.start(&main_handle)
         || ! main_handle.reset(&main_handle)) {
        test_fail();
    }

    if (interrupt_handler_call_count) {
        test_fail();
    }

    if (main_handle.get_counter(&main_handle, &counter) || counter) {
        test_fail();
    }

    dispose_hook_called = false;

    // --- main handle allocate once again ---
    if (driver.channel_handle_register(&driver, &main_handle, MAIN, (dispose_function_t) test_dispose_hook)) {
        test_fail();
    }

    if (main_handle.vector
                .register_handler(&main_handle.vector, (void (*)(void *)) test_interrupt_handler, &pcb) == NULL) {
        test_fail();
    }

    if (main_vector_content_backup == __vector(main_vector_no)) {
        test_fail();
    }

    // --- main handle trigger ---
    if (interrupt_handler_call_count) {
        test_fail();
    }

    if (main_handle.vector.trigger(&main_handle.vector)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 1) {
        test_fail();
    }

    interrupt_handler_call_count = 0;

    // --- shared handle 1 allocate ---
    if (driver.channel_handle_register(&driver, &shared_handle_1, SHARED, (dispose_function_t) test_dispose_hook)) {
        test_fail();
    }

    if (shared_handle_1.vector
                .register_handler(&shared_handle_1.vector, (void (*)(void *)) test_interrupt_handler, &pcb) == NULL) {
        test_fail();
    }

    if (shared_vector_content_backup == __vector(shared_vector_no)) {
        test_fail();
    }

    // --- shared handle 1 trigger ---
    if (interrupt_handler_call_count) {
        test_fail();
    }

    if (shared_handle_1.vector.trigger(&shared_handle_1.vector)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 1) {
        test_fail();
    }

    if (shared_handle_1.vector.trigger(&shared_handle_1.vector)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 2) {
        test_fail();
    }

    // --- reset ---
    if (shared_handle_1.reset(&shared_handle_1)) {
        test_fail();
    }

    // --- timer counter ---
    if (shared_handle_1.get_counter(&shared_handle_1, &counter) || counter) {
        test_fail();
    }

    interrupt_handler_call_count = 0;

    // --- shared handle 2 allocate ---
    if (driver.channel_handle_register(&driver, &shared_handle_2, SHARED, (dispose_function_t) test_dispose_hook)) {
        test_fail();
    }

    if (shared_handle_2.vector
                .register_handler(&shared_handle_2.vector, (void (*)(void *)) test_interrupt_handler, &pcb) == NULL) {
        test_fail();
    }

    if (shared_vector_content_backup == __vector(shared_vector_no)) {
        test_fail();
    }

    // --- shared handle 2 trigger ---
    if (interrupt_handler_call_count) {
        test_fail();
    }

    if (shared_handle_2.vector.trigger(&shared_handle_2.vector)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 1) {
        test_fail();
    }

    if (shared_handle_2.vector.trigger(&shared_handle_2.vector)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 2) {
        test_fail();
    }

    // --- reset ---
    if (shared_handle_2.reset(&shared_handle_2)) {
        test_fail();
    }

    // --- timer counter ---
    if (shared_handle_2.get_counter(&shared_handle_2, &counter) || counter) {
        test_fail();
    }

    interrupt_handler_call_count = 0;

    // --- shared handle allocate when no more handles available ---
    if (driver.channel_handle_register(&driver, &dummy_handle, SHARED,
               (dispose_function_t) test_dispose_hook) != TIMER_NO_HANDLE_AVAILABLE) {

        test_fail();
    }

    // --- shared handle 1 dispose ---
    if (dispose_hook_called) {
        test_fail();
    }

    dispose(&shared_handle_1);

    if ( ! dispose_hook_called) {
        test_fail();
    }

    if (shared_vector_content_backup == __vector(shared_vector_no)) {
        test_fail();
    }

    if ( ! shared_handle_1.vector.trigger(&shared_handle_1.vector) || ! shared_handle_1.start(&shared_handle_1)
         || ! shared_handle_1.reset(&shared_handle_1)) {
        test_fail();
    }

    if (interrupt_handler_call_count) {
        test_fail();
    }

    if (shared_handle_1.get_counter(&shared_handle_1, &counter) || counter) {
        test_fail();
    }

    dispose_hook_called = false;

    // --- shared handle 1 allocate once again ---
    if (driver.channel_handle_register(&driver, &shared_handle_1, SHARED, (dispose_function_t) test_dispose_hook)) {
        test_fail();
    }

    if (shared_handle_1.vector
                .register_handler(&shared_handle_1.vector, (void (*)(void *)) test_interrupt_handler, &pcb) == NULL) {
        test_fail();
    }

    // --- shared handle 1 trigger ---
    if (interrupt_handler_call_count) {
        test_fail();
    }

    if (shared_handle_1.vector.trigger(&shared_handle_1.vector)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 1) {
        test_fail();
    }

    interrupt_handler_call_count = 0;

    // --- shared handle allocate when no more handles available ---
    if (driver.channel_handle_register(&driver, &dummy_handle, SHARED,
               (dispose_function_t) test_dispose_hook) != TIMER_NO_HANDLE_AVAILABLE) {

        test_fail();
    }

    // --- overflow handle allocate ---
    if (driver.channel_handle_register(&driver, &overflow_handle, OVERFLOW, (dispose_function_t) test_dispose_hook)) {
        test_fail();
    }

    if (overflow_handle.vector
                .register_handler(&overflow_handle.vector, (void (*)(void *)) test_interrupt_handler, &pcb) == NULL) {
        test_fail();
    }

    // --- overflow handle trigger ---
    if (interrupt_handler_call_count) {
        test_fail();
    }

    if (overflow_handle.vector.trigger(&overflow_handle.vector)) {
        test_fail();
    }

    if (interrupt_handler_call_count) {
        test_fail();
    }

    // interrupt should should fire once enabled
    if (overflow_handle.vector.set_enabled(&overflow_handle.vector, true)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 1) {
        test_fail();
    }

    if (overflow_handle.vector.trigger(&overflow_handle.vector)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 2) {
        test_fail();
    }

    if (overflow_handle.vector.trigger(&overflow_handle.vector)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 3) {
        test_fail();
    }

    // --- reset ---
    if (overflow_handle.reset(&overflow_handle)) {
        test_fail();
    }

    // --- timer counter ---
    if (overflow_handle.get_counter(&overflow_handle, &counter) || counter) {
        test_fail();
    }

    // --- overflow handle allocate when allocated already ---
    if (driver.channel_handle_register(&driver, &dummy_handle, OVERFLOW,
               (dispose_function_t) test_dispose_hook) != TIMER_NO_HANDLE_AVAILABLE) {

        test_fail();
    }

    interrupt_handler_call_count = 0;

    // --- overflow handle dispose ---
    if (dispose_hook_called) {
        test_fail();
    }

    dispose(&overflow_handle);

    if ( ! dispose_hook_called) {
        test_fail();
    }

    if (shared_vector_content_backup == __vector(shared_vector_no)) {
        test_fail();
    }

    if ( ! overflow_handle.vector.trigger(&overflow_handle.vector) || ! overflow_handle.start(&overflow_handle)
         || ! overflow_handle.reset(&overflow_handle)) {
        test_fail();
    }

    if (interrupt_handler_call_count) {
        test_fail();
    }

    if (overflow_handle.get_counter(&overflow_handle, &counter) || counter) {
        test_fail();
    }

    dispose_hook_called = false;

    // --- overflow handle allocate once again ---
    if (driver.channel_handle_register(&driver, &overflow_handle, OVERFLOW, (dispose_function_t) test_dispose_hook)) {
        test_fail();
    }

    if (overflow_handle.vector
                .register_handler(&overflow_handle.vector, (void (*)(void *)) test_interrupt_handler, &pcb) == NULL) {
        test_fail();
    }

    // --- overflow handle trigger ---
    if (interrupt_handler_call_count) {
        test_fail();
    }

    if (overflow_handle.vector.set_enabled(&overflow_handle.vector, true)) {
        test_fail();
    }

    if (interrupt_handler_call_count) {
        test_fail();
    }

    if (overflow_handle.vector.trigger(&overflow_handle.vector)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 1) {
        test_fail();
    }

    interrupt_handler_call_count = 0;

    // --- driver dispose ---
    dispose(&driver);

    if ( ! dispose_hook_called) {
        test_fail();
    }

    if ( ! main_handle.start(&main_handle) || ! shared_handle_1.start(&shared_handle_1) ||
         ! overflow_handle.start(&overflow_handle)) {
        test_fail();
    }

    if ( ! main_handle.vector.trigger(&main_handle.vector) || ! shared_handle_1.vector.trigger(&shared_handle_1.vector) ||
         ! overflow_handle.vector.trigger(&overflow_handle.vector)) {
        test_fail();
    }

    if (interrupt_handler_call_count) {
        test_fail();
    }

    if ( ! main_handle.get_counter(&main_handle, &counter) || ! shared_handle_1.get_counter(&shared_handle_1, &counter) ||
            ! overflow_handle.get_counter(&overflow_handle, &counter)) {
        test_fail();
    }

    if (main_vector_content_backup != __vector(main_vector_no)) {
        test_fail();
    }

    if (shared_vector_content_backup != __vector(shared_vector_no)) {
        test_fail();
    }
}

// -------------------------------------------------------------------------------------

static void test_interrupt_handler(Process_control_block_t *handler_param) {
    if (handler_param != &pcb) {
        test_fail();
    }

    interrupt_handler_call_count++;
}

static dispose_function_t test_dispose_hook(Disposable_t *disposable) {
    dispose_hook_called = true;

    return NULL;
}
