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

static volatile uint8_t main_handle_interrupt_handler_call_count;
static volatile uint8_t shared_handle_interrupt_handler_call_count;
static volatile uint8_t overflow_handle_interrupt_handler_call_count;

// -------------------------------------------------------------------------------------

void test_driver_timer_multiple() {

    main_handle_interrupt_handler_call_count = shared_handle_interrupt_handler_call_count
            = overflow_handle_interrupt_handler_call_count = 0;
    uint16_t counter, counter_prev;
    uint8_t i;

    WDT_disable();
    default_clock_setup();

    uint8_t main_vector_no = TIMER_A_VECTOR(__TEST_TIMER_NO__, TIMER_VECTOR_MAIN);
    uint8_t shared_vector_no = TIMER_A_VECTOR(__TEST_TIMER_NO__, TIMER_VECTOR_SHARED);

    timer_config.mode = MC__CONTINOUS;
    timer_config.clock_source = TASSEL__SMCLK;
    timer_config.clock_source_divider = ID__1;
    timer_config.clock_source_divider_expansion = TAIDEX__1;

    __enable_interrupt();

    timer_driver_register(&timer_driver, &timer_config, TIMER_A_BASE(__TEST_TIMER_NO__), main_vector_no, shared_vector_no, 2);

    assert( ! timer_driver_channel_register(&timer_driver, &main_handle, MAIN, NULL)
            && ! timer_driver_channel_register(&timer_driver, &shared_handle_1, SHARED, NULL)
            && ! timer_driver_channel_register(&timer_driver, &overflow_handle, OVERFLOW, NULL));

    assert(vector_register_handler(&main_handle, main_handle_interrupt_handler, &main_handle, NULL)
            && vector_register_handler(&shared_handle_1, shared_handle_interrupt_handler, &shared_handle_1, NULL)
            && vector_register_handler(&overflow_handle, overflow_handle_interrupt_handler, &overflow_handle, NULL));

    assert( ! main_handle_interrupt_handler_call_count && ! shared_handle_interrupt_handler_call_count
            && ! overflow_handle_interrupt_handler_call_count);

    assert( ! timer_channel_get_counter(&main_handle, &counter) && ! counter);
    assert( ! timer_channel_get_counter(&shared_handle_1, &counter) && ! counter);
    assert( ! timer_channel_get_counter(&overflow_handle, &counter) && ! counter);

    // --- start overflow handle, do not expect interrupt on others ---
    assert( ! timer_channel_start(&overflow_handle));

    while( ! overflow_handle_interrupt_handler_call_count);

    assert( ! main_handle_interrupt_handler_call_count && ! shared_handle_interrupt_handler_call_count);

    assert( ! timer_channel_reset(&overflow_handle));

    // --- test that trigger actually works on stopped handles ---
    assert( ! vector_trigger(&main_handle));

    assert(main_handle_interrupt_handler_call_count == 1 && ! shared_handle_interrupt_handler_call_count
            && overflow_handle_interrupt_handler_call_count == 1);

    assert( ! vector_trigger(&shared_handle_1));

    assert(main_handle_interrupt_handler_call_count == 1 && shared_handle_interrupt_handler_call_count == 1
            && overflow_handle_interrupt_handler_call_count == 1);

    // --- test counter read ---
    assert( ! timer_channel_get_counter(&overflow_handle, &counter) && counter);
    assert( ! timer_channel_stop(&overflow_handle));

    counter_prev = counter;

    assert( ! timer_channel_get_counter(&overflow_handle, &counter) && counter && counter != counter_prev);

    // --- test counting stopped ---
    counter_prev = counter;

    assert( ! timer_channel_get_counter(&overflow_handle, &counter) &&  counter && counter == counter_prev);
    assert( ! timer_channel_reset(&overflow_handle));

    // --- test reset ---
    assert( ! timer_channel_get_counter(&overflow_handle, &counter) && ! counter);

    counter_prev = counter = 0;

    main_handle_interrupt_handler_call_count = shared_handle_interrupt_handler_call_count
            = overflow_handle_interrupt_handler_call_count = 0;

    // --- capture mode test ---
    timer_channel_set_capture_mode(&main_handle, CM__BOTH, CCIS__GND, SCS__SYNC);

    assert( ! timer_channel_start(&main_handle));

    assert( ! main_handle_interrupt_handler_call_count && ! shared_handle_interrupt_handler_call_count
            && ! overflow_handle_interrupt_handler_call_count);

    assert( ! timer_channel_get_counter(&main_handle, &counter) && counter);
    assert( ! timer_channel_is_capture_overflow_set(&main_handle));
    assert( ! timer_channel_get_capture_value(&main_handle));

    timer_channel_set_capture_mode(&main_handle, CM__BOTH, CCIS__VCC, SCS__SYNC);

    assert(main_handle_interrupt_handler_call_count == 1 && ! shared_handle_interrupt_handler_call_count
            && ! overflow_handle_interrupt_handler_call_count);

    assert(timer_channel_get_capture_value(&main_handle));
    assert( ! timer_channel_is_capture_overflow_set(&main_handle));

    // --- capture overflow test ---
    timer_channel_set_capture_mode(&main_handle, CM__BOTH, CCIS__GND, SCS__SYNC);
    timer_channel_set_capture_mode(&main_handle, CM__BOTH, CCIS__VCC, SCS__SYNC);

    assert(timer_channel_is_capture_overflow_set(&main_handle));
    assert( ! timer_channel_is_capture_overflow_set(&main_handle));

    // --- start overflow handle again ---
    assert( ! timer_channel_start(&overflow_handle));

    assert(main_handle_interrupt_handler_call_count == 3 && ! shared_handle_interrupt_handler_call_count
        && ! overflow_handle_interrupt_handler_call_count);

    assert(timer_channel_reset(&main_handle) && timer_channel_reset(&overflow_handle));

    while( ! overflow_handle_interrupt_handler_call_count);

    assert(main_handle_interrupt_handler_call_count == 3 && !shared_handle_interrupt_handler_call_count
            && overflow_handle_interrupt_handler_call_count == 1);

    // --- test all triggers when started ---
    assert( ! vector_trigger(&shared_handle_1));

    assert(main_handle_interrupt_handler_call_count == 3 && shared_handle_interrupt_handler_call_count == 1
            && overflow_handle_interrupt_handler_call_count == 1);

    assert( ! vector_trigger(&main_handle));

    assert(main_handle_interrupt_handler_call_count == 4 && shared_handle_interrupt_handler_call_count == 1
            && overflow_handle_interrupt_handler_call_count == 1);

    // --- overflow handle should trigger only when started ---
    assert( ! vector_trigger(&overflow_handle));

    assert(main_handle_interrupt_handler_call_count == 4 && shared_handle_interrupt_handler_call_count == 1
            && overflow_handle_interrupt_handler_call_count == 2);

    // --- compare mode test ---
    timer_channel_set_compare_mode(&main_handle, NULL);
    timer_channel_set_compare_mode(&shared_handle_1, NULL);

    assert(main_handle_interrupt_handler_call_count == 4 && shared_handle_interrupt_handler_call_count == 1
            && overflow_handle_interrupt_handler_call_count == 2);

    assert( ! timer_channel_reset(&overflow_handle));

    assert(main_handle_interrupt_handler_call_count == 4 && shared_handle_interrupt_handler_call_count == 1
            && overflow_handle_interrupt_handler_call_count == 2);

    main_handle_interrupt_handler_call_count = shared_handle_interrupt_handler_call_count
            = overflow_handle_interrupt_handler_call_count = 0;

    // --- assume counter > 0 already ---
    timer_channel_start(&main_handle);
    timer_channel_start(&shared_handle_1);

    assert( ! main_handle_interrupt_handler_call_count && ! shared_handle_interrupt_handler_call_count
            && ! overflow_handle_interrupt_handler_call_count);

    timer_channel_set_compare_value(&main_handle, 0x5555);
    timer_channel_set_compare_value(&shared_handle_1, 0xAAAA);

    assert( ! main_handle_interrupt_handler_call_count && ! shared_handle_interrupt_handler_call_count
                && ! overflow_handle_interrupt_handler_call_count);

    for (i = 0; i < 2; i++) {
        while (main_handle_interrupt_handler_call_count <= i);

        assert(main_handle_interrupt_handler_call_count == i + 1 && shared_handle_interrupt_handler_call_count == i
                && overflow_handle_interrupt_handler_call_count == i);

        while (shared_handle_interrupt_handler_call_count <= i);

        assert(main_handle_interrupt_handler_call_count == i + 1 && shared_handle_interrupt_handler_call_count == i + 1
                && overflow_handle_interrupt_handler_call_count == i);

        while (overflow_handle_interrupt_handler_call_count <= i);

        assert(main_handle_interrupt_handler_call_count == i + 1 && shared_handle_interrupt_handler_call_count == i + 1
                   && overflow_handle_interrupt_handler_call_count == i + 1);
    }

    dispose(&main_handle);
    dispose(&overflow_handle);

    assert( ! timer_channel_reset(&shared_handle_1));

    // --- test the counter is still active ---
    assert( ! timer_channel_get_counter(&shared_handle_1, &counter));

    counter_prev = counter;

    assert( ! timer_channel_get_counter(&shared_handle_1, &counter));

    assert_not(counter_prev == counter);

    assert( ! timer_channel_stop(&shared_handle_1));

    // --- test the counter is inactive ---
    assert( ! timer_channel_get_counter(&shared_handle_1, &counter));

    counter_prev = counter;

    assert( ! timer_channel_get_counter(&shared_handle_1, &counter));

    assert(counter_prev == counter);

    dispose(&timer_driver);
}

static void main_handle_interrupt_handler(Timer_channel_handle_t *handle) {
    assert(handle == &main_handle);

    main_handle_interrupt_handler_call_count++;
}

static void shared_handle_interrupt_handler(Timer_channel_handle_t *handle) {
    assert(handle == &shared_handle_1);

    shared_handle_interrupt_handler_call_count++;
}

static void overflow_handle_interrupt_handler(Timer_channel_handle_t *handle) {
    assert(handle == &overflow_handle);

    overflow_handle_interrupt_handler_call_count++;
}
