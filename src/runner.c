// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/runner.h>
#include <compiler.h>

// -------------------------------------------------------------------------------------

volatile __persistent uint16_t last_executed_test_id = 0;
volatile __persistent uint16_t last_passed_test_id = 0;

// -------------------------------------------------------------------------------------

#define TEST_CNT    24

void (*tests_all[TEST_CNT])(void) = {
    test_driver_wdt_disabled,
    test_driver_wdt_watchdog,
    test_driver_wdt_interval,
    test_driver_vector_trigger,
    test_driver_timer_dispose,
    test_driver_timer_multiple,
    test_driver_io_handler,
    test_driver_eusci_uart,
    test_driver_eusci_spi,
    test_driver_dma_transfer,
    test_driver_dma_uart,
    test_driver_crc_consumer,
    test_driver_stack_pointer,
    test_driver_stack_deferred,
    test_kernel_dispose_chain,
    test_kernel_dispose_resource,
    test_kernel_process_lifecycle,
    test_kernel_queue_sorting,
    test_kernel_queue_fifo,
    test_kernel_queue_priority,
    test_kernel_sync_barrier,
    test_kernel_sync_inversion,
    test_kernel_event_trigger,
    test_kernel_event_inheritance,
};

// -------------------------------------------------------------------------------------

void test_runner_all() {
    volatile uint16_t i, j;

    IO_debug_init();

    if (last_executed_test_id != last_passed_test_id) {
        last_executed_test_id = last_passed_test_id = 0;

        assert(false);
    }

    if (last_executed_test_id == TEST_CNT) {
        last_executed_test_id = last_passed_test_id = 0;

        WDT_disable();
        default_clock_setup(0, DCOFSEL_0, DIVM__1);
        IO_debug_unlock();
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
