// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/standalone/timing.h>
#include <kernel.h>


#define __INIT_PROCESS_PRIORITY__               0
#define __TEST_PROCESS_1_PRIORITY__             1
#define __TEST_TIMED_SIGNAL_1_PRIORITY__        2
#define __TEST_TIMED_SIGNAL_2_PRIORITY__        3

#define __TEST_TIMER_B_NO__                     0

// -------------------------------------------------------------------------------------

static signal_t test_process_1_entry_point(Semaphore_t *);

static volatile bool process_1_started;

static  bool led_toggle_handler_1(Timed_signal_t *owner, signal_t);
static  bool led_toggle_handler_2(Timed_signal_t *owner, signal_t);

// -------------------------------------------------------------------------------------

static uint32_t mul_4(uint32_t value) {
    return value * 4;
}

static uint32_t div_4(uint32_t value) {
    return value / 4;
}

static uint32_t mul_256(uint32_t value) {
    return value * 256;
}

static uint32_t div_256(uint32_t value) {
    return value / 256;
}

// -------------------------------------------------------------------------------------

static void init() {
    // initialize test process
    zerofill(&process_1.create_config);

    process_1.create_config.priority = __TEST_PROCESS_1_PRIORITY__;
    process_1.create_config.stack_addr_low = (data_pointer_register_t) test_stack_1;
    process_1.create_config.stack_size = __TEST_STACK_SIZE__;
    process_1.create_config.entry_point = (process_entry_point_t) test_process_1_entry_point;
    process_1.create_config.arg_1 = &semaphore_1;

    // --- create process with higher priority than init process ---
    process_create(&process_1);

    // initialize test semaphore
    semaphore_create(&semaphore_1, 0);
}

void test_kernel_standalone_timing() {

    Timer_config_t config;
    volatile uint32_t cnt;

#ifndef __SIGNAL_PROCESSOR_DISABLE__
    WDT_disable();

    IO_debug_init();
    IO_debug_unlock();

    bool alternative_handle = false;
    process_1_started = false;

    // --- default timing_handle_1 init - timer_A, input SMCLK / 1, bit width 16 ---
    default_timer_init(&timer_driver_1, _timer_channel_handle_(&context_switch_handle), _timer_channel_handle_(&timing_handle_1), NULL);

    timing_handle_1.timer_counter_bit_width = 16;
    timing_handle_1.usecs_to_ticks = div_4;
    timing_handle_1.ticks_to_usecs = mul_4;

    // DCO 8MHz, SMCLK input DCO / 32
    default_clock_setup(0, DCOFSEL_6, DIVM__32);

    // --- timing_handle_2 init - timer_B, input SMCLK / 64, bit width 10 ---
    uint8_t main_vector_no = TIMER_B_VECTOR(__TEST_TIMER_B_NO__, TIMER_VECTOR_MAIN);
    uint8_t shared_vector_no = TIMER_B_VECTOR(__TEST_TIMER_B_NO__, TIMER_VECTOR_SHARED);

    config.mode = MC__CONTINOUS;
    // SMCLK, counter register bit width 10
    config.clock_source = TASSEL__SMCLK | CNTL__10;
    // SMCLK divide by 8
    config.clock_source_divider = ID__8;
    // extended SMCLK divide by 8
    config.clock_source_divider_expansion = TAIDEX__8;

    timer_driver_register(&timer_driver_2, &config, TIMER_B_BASE(__TEST_TIMER_B_NO__), main_vector_no, shared_vector_no, 2);
    timer_driver_channel_register(&timer_driver_2, &timing_handle_2, SHARED, NULL);

    timing_handle_2.timer_counter_bit_width = 10;
    timing_handle_2.usecs_to_ticks = div_256;
    timing_handle_2.ticks_to_usecs = mul_256;

    // --- create and schedule process from current context ---
    assert( ! kernel_start(&init_process, 0, init, false, (Context_switch_handle_t *) &context_switch_handle, NULL));

    // --- late timing init ---
    timing_reinit(&timing_handle_2, &signal_processor, true);

    // -------------------------------------------------------------------------------------

    // --- test periodic signal 1 ---
    timed_signal_create(&timed_signal_1, led_toggle_handler_1, true, &(Schedule_config_t){__TEST_TIMED_SIGNAL_1_PRIORITY__});
    timed_signal_set_delay_secs(&timed_signal_1, 0, 16, 977);
    timed_signal_schedule(&timed_signal_1);

    // --- test periodic signal 2 ---
    timed_signal_create(&timed_signal_2, led_toggle_handler_2, false, &(Schedule_config_t){__TEST_TIMED_SIGNAL_2_PRIORITY__});
    timed_signal_set_delay_secs(&timed_signal_2, 0, 17, 0);
    timed_signal_schedule(&timed_signal_2);

    // --- schedule test process ---
    process_schedule(&process_1, 0);

    assert(process_1_started);

    while (true) {
        // wait ~7 seconds (at 8MHz)
        for (cnt = 0; cnt < 2900000; cnt++);

        // --- test handle dynamic switch ---
        timing_reinit(alternative_handle ? &timing_handle_2 : &timing_handle_1, &signal_processor, false);

        alternative_handle = ! alternative_handle;

        IO_debug_pin_4_toggle();
        IO_debug_pin_4_toggle();
    }
#endif

}

static signal_t test_process_1_entry_point(Semaphore_t *semaphore) {
    assert(semaphore == &semaphore_1);

#ifndef __SIGNAL_PROCESSOR_DISABLE__
    process_1_started = true;

    // preset delay to be used for blocking calls
    timeout_secs(0, 15, 155);

    IO_green_led_toggle();

    IO_debug_pin_2_toggle();

    while (true) {
        // --- sleep test ---
        assert(sleep_cached() == TIMING_SIGNAL_TIMEOUT);

        IO_green_led_toggle();

        IO_debug_pin_2_toggle();

        // --- blocking API with timeout test ---
        assert(semaphore_acquire(semaphore, timeout_cached()) == SEMAPHORE_ACQUIRE_TIMEOUT);

        IO_green_led_toggle();

        IO_debug_pin_2_toggle();
    }
#endif

    return 0;
}

static bool led_toggle_handler_1(Timed_signal_t *owner, signal_t signal) {

    assert(owner == &timed_signal_1);
    assert(signal == TIMING_SIGNAL_TIMEOUT);

    IO_red_led_toggle();

    IO_debug_pin_1_toggle();

    return true;
}

static bool led_toggle_handler_2(Timed_signal_t *owner, signal_t signal) {

    assert(owner == &timed_signal_2);
    assert(signal == TIMING_SIGNAL_TIMEOUT);

    if ( ! timed_signal_is_periodic(owner)) {
        timed_signal_set_periodic(&timed_signal_2, true);
    }

    IO_red_led_toggle();
    IO_green_led_toggle();

    IO_debug_pin_3_toggle();

    return true;
}
