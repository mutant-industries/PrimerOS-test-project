// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/timer/dispose.h>
#include <driver/timer.h>
#include <process.h>

// -------------------------------------------------------------------------------------

#define __TEST_TIMER_NO__   1

static void test_interrupt_handler(Timer_driver_t *);
static dispose_function_t test_dispose_hook(Disposable_t *disposable);

static volatile uint8_t interrupt_handler_call_count;
static volatile bool dispose_hook_called;

// -------------------------------------------------------------------------------------

void test_driver_timer_dispose() {
    uint16_t main_vector_content_backup, shared_vector_content_backup, counter;

    dispose_hook_called = false;
    interrupt_handler_call_count = 0;

    WDT_disable();
    default_clock_setup(0, DCOFSEL_0, DIVM__1);

    uint8_t main_vector_no = TIMER_A_VECTOR(__TEST_TIMER_NO__, TIMER_VECTOR_MAIN);
    uint8_t shared_vector_no = TIMER_A_VECTOR(__TEST_TIMER_NO__, TIMER_VECTOR_SHARED);

    main_vector_content_backup = __vector(main_vector_no);
    shared_vector_content_backup = __vector(shared_vector_no);

    timer_config.mode = MC__CONTINOUS;
    timer_config.clock_source = TASSEL__SMCLK;
    timer_config.clock_source_divider = ID__1;
    timer_config.clock_source_divider_expansion = TAIDEX__1;

    __enable_interrupt();

    timer_driver_register(&timer_driver_1, &timer_config, TIMER_A_BASE(__TEST_TIMER_NO__), main_vector_no, shared_vector_no, 3);

    // --- main handle allocate ---
    assert( ! timer_driver_channel_register(&timer_driver_1, &main_handle, MAIN, (dispose_function_t) test_dispose_hook));

    assert(vector_register_handler(&main_handle, test_interrupt_handler, &timer_driver_1, NULL));

    assert(main_vector_content_backup != __vector(main_vector_no));

    // --- main handle trigger ---
    assert( ! interrupt_handler_call_count);

    assert( ! vector_trigger(&main_handle));

    assert(interrupt_handler_call_count == 1);

    assert( ! vector_trigger(&main_handle));

    assert(interrupt_handler_call_count == 2);

    // --- reset ---
    assert( ! timer_channel_reset(&main_handle));

    // --- timer counter ---
    assert( ! timer_channel_get_counter(&main_handle, &counter) && ! counter);

    // --- main handle allocate when allocated already ---
    assert( ! timer_driver_channel_register(&timer_driver_1, &dummy_handle, MAIN,
               (dispose_function_t) test_dispose_hook) != TIMER_NO_HANDLE_AVAILABLE);

    interrupt_handler_call_count = 0;

    // --- main handle dispose ---
    assert_not(dispose_hook_called);

    dispose(&main_handle);

    assert(dispose_hook_called);
    assert(main_vector_content_backup == __vector(main_vector_no));

    assert(vector_trigger(&main_handle) && timer_channel_start(&main_handle) && timer_channel_reset(&main_handle));

    assert( ! interrupt_handler_call_count);
    assert( ! timer_channel_get_counter(&main_handle, &counter) && ! counter);

    dispose_hook_called = false;

    // --- main handle allocate once again ---
    assert( ! timer_driver_channel_register(&timer_driver_1, &main_handle, MAIN, (dispose_function_t) test_dispose_hook));

    assert(vector_register_handler(&main_handle, test_interrupt_handler, &timer_driver_1, NULL));

    assert_not(main_vector_content_backup == __vector(main_vector_no));

    // --- main handle trigger ---
    assert( ! interrupt_handler_call_count);

    assert( ! vector_trigger(&main_handle));

    assert(interrupt_handler_call_count == 1);

    interrupt_handler_call_count = 0;

    // --- shared handle 1 allocate ---
    assert( ! timer_driver_channel_register(&timer_driver_1, &shared_handle_1, SHARED, (dispose_function_t) test_dispose_hook));

    assert(vector_register_handler(&shared_handle_1, test_interrupt_handler, &timer_driver_1, NULL));

    assert_not(shared_vector_content_backup == __vector(shared_vector_no));

    // --- shared handle 1 trigger ---
    assert( ! interrupt_handler_call_count);

    assert( ! vector_trigger(&shared_handle_1));

    assert(interrupt_handler_call_count == 1);

    assert( ! vector_trigger(&shared_handle_1));

    assert(interrupt_handler_call_count == 2);

    // --- reset ---
    assert( ! timer_channel_reset(&shared_handle_1));

    // --- timer counter ---
    assert( ! timer_channel_get_counter(&shared_handle_1, &counter) && ! counter);

    interrupt_handler_call_count = 0;

    // --- shared handle 2 allocate ---
    assert( ! timer_driver_channel_register(&timer_driver_1, &shared_handle_2, SHARED, (dispose_function_t) test_dispose_hook));

    assert(vector_register_handler(&shared_handle_2, test_interrupt_handler, &timer_driver_1, NULL));

    assert_not(shared_vector_content_backup == __vector(shared_vector_no));

    // --- shared handle 2 trigger ---
    assert( ! interrupt_handler_call_count);

    assert( ! vector_trigger(&shared_handle_2));

    assert(interrupt_handler_call_count == 1);

    assert( ! vector_trigger(&shared_handle_2));

    assert(interrupt_handler_call_count == 2);

    // --- reset ---
    assert( ! timer_channel_reset(&shared_handle_2));

    // --- timer counter ---
    assert( ! timer_channel_get_counter(&shared_handle_2, &counter) && ! counter);

    interrupt_handler_call_count = 0;

    // --- shared handle allocate when no more handles available ---
    assert( ! timer_driver_channel_register(&timer_driver_1, &dummy_handle, SHARED,
               (dispose_function_t) test_dispose_hook) != TIMER_NO_HANDLE_AVAILABLE);

    // --- shared handle 1 dispose ---
    assert_not(dispose_hook_called);

    dispose(&shared_handle_1);

    assert(dispose_hook_called);
    assert_not(shared_vector_content_backup == __vector(shared_vector_no));

    assert(vector_trigger(&shared_handle_1) && timer_channel_start(&shared_handle_1) && timer_channel_reset(&shared_handle_1));

    assert( ! interrupt_handler_call_count);

    assert( ! timer_channel_get_counter(&shared_handle_1, &counter) && ! counter);

    dispose_hook_called = false;

    // --- shared handle 1 allocate once again ---
    assert( ! timer_driver_channel_register(&timer_driver_1, &shared_handle_1, SHARED, (dispose_function_t) test_dispose_hook));

    assert(vector_register_handler(&shared_handle_1, test_interrupt_handler, &timer_driver_1, NULL));

    // --- shared handle 1 trigger ---
    assert( ! interrupt_handler_call_count);

    assert( ! vector_trigger(&shared_handle_1));

    assert(interrupt_handler_call_count == 1);

    interrupt_handler_call_count = 0;

    // --- shared handle allocate when no more handles available ---
    assert( ! timer_driver_channel_register(&timer_driver_1, &dummy_handle, SHARED,
               (dispose_function_t) test_dispose_hook) != TIMER_NO_HANDLE_AVAILABLE);

    // --- overflow handle allocate ---
    assert( ! timer_driver_channel_register(&timer_driver_1, &overflow_handle, OVERFLOW, (dispose_function_t) test_dispose_hook));

    assert(vector_register_handler(&overflow_handle, test_interrupt_handler, &timer_driver_1, NULL));

    // --- overflow handle trigger ---
    assert( ! interrupt_handler_call_count);

    assert( ! vector_trigger(&overflow_handle));

    assert( ! interrupt_handler_call_count);

    // interrupt should should fire once enabled
    assert( ! vector_set_enabled(&overflow_handle, true));

    assert(interrupt_handler_call_count == 1);

    assert( ! vector_trigger(&overflow_handle));

    assert(interrupt_handler_call_count == 2);

    assert( ! vector_trigger(&overflow_handle));

    assert(interrupt_handler_call_count == 3);

    // --- reset ---
    assert( ! timer_channel_reset(&overflow_handle));

    // --- timer counter ---
    assert( ! timer_channel_get_counter(&overflow_handle, &counter) && ! counter);

    // --- overflow handle allocate when allocated already ---
    assert( ! timer_driver_channel_register(&timer_driver_1, &dummy_handle, OVERFLOW,
               (dispose_function_t) test_dispose_hook) != TIMER_NO_HANDLE_AVAILABLE);

    interrupt_handler_call_count = 0;

    // --- overflow handle dispose ---
    assert_not(dispose_hook_called);

    dispose(&overflow_handle);

    assert(dispose_hook_called);
    assert_not(shared_vector_content_backup == __vector(shared_vector_no));

    assert(vector_trigger(&overflow_handle) && timer_channel_start(&overflow_handle) && timer_channel_reset(&overflow_handle));

    assert( ! interrupt_handler_call_count);

    assert( ! timer_channel_get_counter(&overflow_handle, &counter) && ! counter);

    dispose_hook_called = false;

    // --- overflow handle allocate once again ---
    assert( ! timer_driver_channel_register(&timer_driver_1, &overflow_handle, OVERFLOW, (dispose_function_t) test_dispose_hook));

    assert(vector_register_handler(&overflow_handle, test_interrupt_handler, &timer_driver_1, NULL));

    // --- overflow handle trigger ---
    assert( ! interrupt_handler_call_count);

    assert( ! vector_set_enabled(&overflow_handle, true));

    assert( ! interrupt_handler_call_count);

    assert( ! vector_trigger(&overflow_handle));

    assert(interrupt_handler_call_count == 1);

    interrupt_handler_call_count = 0;

    // --- driver dispose ---
    dispose(&timer_driver_1);

    assert(dispose_hook_called);

    assert(timer_channel_start(&main_handle) && timer_channel_start(&shared_handle_1) && timer_channel_start(&overflow_handle));

    assert(vector_trigger(&main_handle) && vector_trigger(&shared_handle_1) && vector_trigger(&overflow_handle));

    assert( ! interrupt_handler_call_count);

    assert(timer_channel_get_counter(&main_handle, &counter) && timer_channel_get_counter(&shared_handle_1, &counter) &&
            timer_channel_get_counter(&overflow_handle, &counter));

    assert(main_vector_content_backup == __vector(main_vector_no));
    assert(shared_vector_content_backup == __vector(shared_vector_no));
}

// -------------------------------------------------------------------------------------

static void test_interrupt_handler(Timer_driver_t *handler_param) {
    assert(handler_param == &timer_driver_1);

    interrupt_handler_call_count++;
}

static dispose_function_t test_dispose_hook(Disposable_t *disposable) {
    dispose_hook_called = true;

    return NULL;
}
