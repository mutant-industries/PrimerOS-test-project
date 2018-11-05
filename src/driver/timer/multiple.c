// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/timer/multiple.h>
#include <driver/timer.h>
#include <process.h>

// -------------------------------------------------------------------------------------

#define __TEST_TIMER_NO__   2

static void main_handle_interrupt_handler(Timer_channel_handle_t *);
static void shared_handle_interrupt_handler(Timer_channel_handle_t *);
static void overflow_handle_interrupt_handler(Timer_channel_handle_t *);

static Timer_config_t config;
static Timer_driver_t driver;

static Timer_channel_handle_t main_handle;
static Timer_channel_handle_t shared_handle;
static Timer_channel_handle_t overflow_handle;

static Process_control_block_t pcb;
static volatile uint8_t main_handle_interrupt_handler_call_count;
static volatile uint8_t shared_handle_interrupt_handler_call_count;
static volatile uint8_t overflow_handle_interrupt_handler_call_count;

// -------------------------------------------------------------------------------------

void test_driver_timer_multiple() {

    running_process = &pcb;
    main_handle_interrupt_handler_call_count = shared_handle_interrupt_handler_call_count
            = overflow_handle_interrupt_handler_call_count = 0;
    uint16_t counter, counter_prev;
    uint8_t i;

    WDT_disable();
    default_clock_setup();

    uint8_t main_vector_no = TIMER_A_VECTOR(__TEST_TIMER_NO__, TIMER_VECTOR_MAIN);
    uint8_t shared_vector_no = TIMER_A_VECTOR(__TEST_TIMER_NO__, TIMER_VECTOR_SHARED);

    config.mode = MC__CONTINOUS;
    config.clock_source = TASSEL__SMCLK;
    config.clock_source_divider = ID__1;
    config.clock_source_divider_expansion = TAIDEX__1;

    __enable_interrupt();

    timer_driver_register(&driver, &config, TIMER_A_BASE(__TEST_TIMER_NO__), main_vector_no, shared_vector_no, 2);

    if (driver.channel_handle_register(&driver, &main_handle, MAIN, NULL)
            || driver.channel_handle_register(&driver, &shared_handle, SHARED, NULL)
            || driver.channel_handle_register(&driver, &overflow_handle, OVERFLOW, NULL)) {

        test_fail();
    }

    if (main_handle.vector.register_handler(&main_handle.vector, (void (*)(void *)) main_handle_interrupt_handler, &main_handle) == NULL
            || shared_handle.vector.register_handler(&shared_handle.vector, (void (*)(void *)) shared_handle_interrupt_handler, &shared_handle) == NULL
            || overflow_handle.vector.register_handler(&overflow_handle.vector, (void (*)(void *)) overflow_handle_interrupt_handler, &overflow_handle) == NULL) {

        test_fail();
    }

    if (main_handle_interrupt_handler_call_count || shared_handle_interrupt_handler_call_count
            || overflow_handle_interrupt_handler_call_count) {

        test_fail();
    }

    if (main_handle.get_counter(&main_handle, &counter) || counter) {
        test_fail();
    }

    if (shared_handle.get_counter(&shared_handle, &counter) || counter) {
        test_fail();
    }

    if (overflow_handle.get_counter(&overflow_handle, &counter) || counter) {
        test_fail();
    }

    // --- start overflow handle, do not expect interrupt on others ---
    if (overflow_handle.start(&overflow_handle)) {
        test_fail();
    }

    while( ! overflow_handle_interrupt_handler_call_count);

    if (main_handle_interrupt_handler_call_count || shared_handle_interrupt_handler_call_count) {
        test_fail();
    }

    if (overflow_handle.reset(&overflow_handle)) {
        test_fail();
    }

    // --- test that trigger actually works on stopped handles ---
    if (main_handle.vector.trigger(&main_handle.vector)) {
        test_fail();
    }

    if (main_handle_interrupt_handler_call_count != 1 || shared_handle_interrupt_handler_call_count
            || overflow_handle_interrupt_handler_call_count != 1) {
        test_fail();
    }

    if (shared_handle.vector.trigger(&shared_handle.vector)) {
        test_fail();
    }

    if (main_handle_interrupt_handler_call_count != 1 || shared_handle_interrupt_handler_call_count != 1
            || overflow_handle_interrupt_handler_call_count != 1) {
        test_fail();
    }

    // --- test counter read ---
    if (overflow_handle.get_counter(&overflow_handle, &counter) || ! counter) {
        test_fail();
    }

    if (overflow_handle.stop(&overflow_handle)) {
        test_fail();
    }

    counter_prev = counter;

    if (overflow_handle.get_counter(&overflow_handle, &counter) || ! counter || counter == counter_prev) {
        test_fail();
    }

    // --- test counting stopped ---
    counter_prev = counter;

    if (overflow_handle.get_counter(&overflow_handle, &counter) || ! counter || counter != counter_prev) {
        test_fail();
    }

    if (overflow_handle.reset(&overflow_handle)) {
        test_fail();
    }

    // --- test reset ---
    if (overflow_handle.get_counter(&overflow_handle, &counter) || counter) {
        test_fail();
    }

    counter_prev = counter = 0;

    main_handle_interrupt_handler_call_count = shared_handle_interrupt_handler_call_count
            = overflow_handle_interrupt_handler_call_count = 0;

    // --- capture mode test ---
    main_handle.set_capture_mode(&main_handle, CM__BOTH, CCIS__GND, SCS__SYNC);

    if (main_handle.start(&main_handle)) {
        test_fail();
    }

    if (main_handle_interrupt_handler_call_count || shared_handle_interrupt_handler_call_count
            || overflow_handle_interrupt_handler_call_count) {

        test_fail();
    }

    if (main_handle.get_counter(&main_handle, &counter) || ! counter) {
        test_fail();
    }

    if (main_handle.is_capture_overflow_set(&main_handle)) {
        test_fail();
    }

    if (main_handle.get_capture_value(&main_handle)) {
        test_fail();
    }

    main_handle.set_capture_mode(&main_handle, CM__BOTH, CCIS__VCC, SCS__SYNC);

    if (main_handle_interrupt_handler_call_count != 1 || shared_handle_interrupt_handler_call_count
            || overflow_handle_interrupt_handler_call_count) {

        test_fail();
    }

    if ( ! main_handle.get_capture_value(&main_handle)) {
        test_fail();
    }

    if (main_handle.is_capture_overflow_set(&main_handle)) {
        test_fail();
    }

    // --- capture overflow test ---
    main_handle.set_capture_mode(&main_handle, CM__BOTH, CCIS__GND, SCS__SYNC);
    main_handle.set_capture_mode(&main_handle, CM__BOTH, CCIS__VCC, SCS__SYNC);

    if ( ! main_handle.is_capture_overflow_set(&main_handle)) {
        test_fail();
    }

    if (main_handle.is_capture_overflow_set(&main_handle)) {
        test_fail();
    }

    // --- start overflow handle again ---
    if (overflow_handle.start(&overflow_handle)) {
        test_fail();
    }

    if (main_handle_interrupt_handler_call_count != 3 || shared_handle_interrupt_handler_call_count
        || overflow_handle_interrupt_handler_call_count) {

        test_fail();
    }

    if ( ! main_handle.reset(&main_handle) || ! overflow_handle.reset(&overflow_handle)) {
        test_fail();
    }

    while( ! overflow_handle_interrupt_handler_call_count);

    if (main_handle_interrupt_handler_call_count != 3 || shared_handle_interrupt_handler_call_count
        || overflow_handle_interrupt_handler_call_count != 1) {

        test_fail();
    }

    // --- test all triggers when started ---
    if (shared_handle.vector.trigger(&shared_handle.vector)) {
        test_fail();
    }

    if (main_handle_interrupt_handler_call_count != 3 || shared_handle_interrupt_handler_call_count != 1
            || overflow_handle_interrupt_handler_call_count != 1) {

        test_fail();
    }

    if (main_handle.vector.trigger(&main_handle.vector)) {
        test_fail();
    }

    if (main_handle_interrupt_handler_call_count != 4 || shared_handle_interrupt_handler_call_count != 1
            || overflow_handle_interrupt_handler_call_count != 1) {

        test_fail();
    }

    // --- overflow handle should trigger only when started ---
    if (overflow_handle.vector.trigger(&overflow_handle.vector)) {
        test_fail();
    }

    if (main_handle_interrupt_handler_call_count != 4 || shared_handle_interrupt_handler_call_count != 1
            || overflow_handle_interrupt_handler_call_count != 2) {

        test_fail();
    }

    // --- compare mode test ---
    main_handle.set_compare_mode(&main_handle, NULL);
    shared_handle.set_compare_mode(&shared_handle, NULL);

    if (main_handle_interrupt_handler_call_count != 4 || shared_handle_interrupt_handler_call_count != 1
            || overflow_handle_interrupt_handler_call_count != 2) {

        test_fail();
    }

    if (overflow_handle.reset(&overflow_handle)) {
        test_fail();
    }

    if (main_handle_interrupt_handler_call_count != 4 || shared_handle_interrupt_handler_call_count != 1
            || overflow_handle_interrupt_handler_call_count != 2) {

        test_fail();
    }

    main_handle_interrupt_handler_call_count = shared_handle_interrupt_handler_call_count
            = overflow_handle_interrupt_handler_call_count = 0;

    // --- assume counter > 0 already ---
    main_handle.start(&main_handle);
    shared_handle.start(&shared_handle);

    if (main_handle_interrupt_handler_call_count || shared_handle_interrupt_handler_call_count
            || overflow_handle_interrupt_handler_call_count) {

        test_fail();
    }

    main_handle.set_compare_value(&main_handle, 0x5555);
    shared_handle.set_compare_value(&shared_handle, 0xAAAA);

    if (main_handle_interrupt_handler_call_count || shared_handle_interrupt_handler_call_count
           || overflow_handle_interrupt_handler_call_count) {

        test_fail();
    }

    for (i = 0; i < 2; i++) {
        while (main_handle_interrupt_handler_call_count <= i);

        if (main_handle_interrupt_handler_call_count != i + 1 || shared_handle_interrupt_handler_call_count != i
                || overflow_handle_interrupt_handler_call_count != i) {

            test_fail();
        }

        while (shared_handle_interrupt_handler_call_count <= i);

        if (main_handle_interrupt_handler_call_count != i + 1 || shared_handle_interrupt_handler_call_count != i + 1
                || overflow_handle_interrupt_handler_call_count != i) {

            test_fail();
        }

        while (overflow_handle_interrupt_handler_call_count <= i);

        if (main_handle_interrupt_handler_call_count != i + 1 || shared_handle_interrupt_handler_call_count != i + 1
                || overflow_handle_interrupt_handler_call_count != i + 1) {

            test_fail();
        }
    }

    dispose(&main_handle);
    dispose(&overflow_handle);

    if (shared_handle.reset(&shared_handle)) {
        test_fail();
    }

    // --- test the counter is still active ---
    if (shared_handle.get_counter(&shared_handle, &counter)) {
        test_fail();
    }

    counter_prev = counter;

    if (shared_handle.get_counter(&shared_handle, &counter)) {
        test_fail();
    }

    if (counter_prev == counter) {
        test_fail();
    }

    if (shared_handle.stop(&shared_handle)) {
        test_fail();
    }

    // --- test the counter is inactive ---
    if (shared_handle.get_counter(&shared_handle, &counter)) {
        test_fail();
    }

    counter_prev = counter;

    if (shared_handle.get_counter(&shared_handle, &counter)) {
        test_fail();
    }

    if (counter_prev != counter) {
        test_fail();
    }

    dispose(&driver);
}

static void main_handle_interrupt_handler(Timer_channel_handle_t *handle) {
    if (handle != &main_handle) {
        test_fail();
    }

    main_handle_interrupt_handler_call_count++;
}

static void shared_handle_interrupt_handler(Timer_channel_handle_t *handle) {
    if (handle != &shared_handle) {
        test_fail();
    }

    shared_handle_interrupt_handler_call_count++;
}

static void overflow_handle_interrupt_handler(Timer_channel_handle_t *handle) {
    if (handle != &overflow_handle) {
        test_fail();
    }

    overflow_handle_interrupt_handler_call_count++;
}
