// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/runner.h>

// -------------------------------------------------------------------------------------

volatile uint16_t last_executed_test_id __attribute__ ((persistent)) = 0;
volatile uint16_t last_passed_test_id __attribute__ ((persistent))  = 0;

// -------------------------------------------------------------------------------------

#define TEST_CNT    8

void (*tests_all[TEST_CNT])(void) = {
    test_driver_wdt_disabled,
    test_driver_wdt_watchdog,
    test_driver_wdt_interval,
    test_driver_disposable_chain,
    test_driver_disposable_resource,
    test_driver_vector_trigger,
    test_driver_timer_dispose,
    test_driver_timer_multiple,
};

// -------------------------------------------------------------------------------------

void test_runner_all() {
    volatile uint32_t i, j;

    IO_debug_init();

    if (last_executed_test_id != last_passed_test_id) {
        last_executed_test_id = last_passed_test_id = 0;

        test_fail();
    }

    if (last_executed_test_id == TEST_CNT) {
        last_executed_test_id = last_passed_test_id = 0;

        WDT_disable();
        default_clock_setup();
        IO_unlock();
        IO_red_led_off();

        for (i = 2; i > 0; i--) {
            for (j = 25000; j > 0; j--);

            IO_green_led_toggle();
        }

        power_on_reset();
    }

    test_runner_single(tests_all[last_executed_test_id++]);

    last_passed_test_id++;

    test_pass();

    power_on_reset();
}

void test_runner_single(void(*test)(void)) {
    (*test)();
}
