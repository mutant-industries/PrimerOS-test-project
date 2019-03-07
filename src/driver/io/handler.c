// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/io/handler.h>
#include <driver/interrupt.h>

// -------------------------------------------------------------------------------------

static void test_interrupt_handler_P5_2(IO_pin_handle_t *, uint8_t interrupt_pin);
static void test_interrupt_handler_P6_1(IO_pin_handle_t *, uint8_t interrupt_pin);
static void test_interrupt_handler_P7_1(IO_pin_handle_t *, uint8_t interrupt_pin);

static volatile uint8_t interrupt_handler_P5_2_call_count, interrupt_handler_P6_1_call_count, interrupt_handler_P7_1_call_count;

// -------------------------------------------------------------------------------------

void test_driver_io_handler() {

    interrupt_handler_P5_2_call_count = interrupt_handler_P6_1_call_count = interrupt_handler_P7_1_call_count = 0;

    WDT_disable();

    // port 1 driver
    IO_port_driver_create(&IO_port_driver_1, PORT_1);

    // pin P1.2 and pin P1.3 - direction out, output level low
    IO_port_handle_register(&IO_port_driver_1, &IO_pin_handle_1, PIN_2 | PIN_3);
    IO_pin_handle_reg_reset(&IO_pin_handle_1, OUT);
    IO_pin_handle_reg_set(&IO_pin_handle_1, DIR);

    // port 5 driver
    IO_port_driver_create(&IO_port_driver_2, PORT_5);

    // pin P5.2 direction in, pull-down resistor enable, interrupt edge low-to-high
    IO_port_handle_register(&IO_port_driver_2, &IO_pin_handle_2, PIN_2);
    IO_pin_handle_reg_reset(&IO_pin_handle_2, DIR);
    IO_pin_handle_reg_reset(&IO_pin_handle_2, OUT);
    IO_pin_handle_reg_set(&IO_pin_handle_2, REN);
    IO_pin_handle_reg_reset(&IO_pin_handle_2, IES);

    // port 6 driver
    IO_port_driver_create(&IO_port_driver_3, PORT_6);

    // pin P6.1 direction in, pull-down resistor enable, interrupt edge high-to-low
    IO_port_handle_register(&IO_port_driver_3, &IO_pin_handle_3, PIN_1);
    IO_pin_handle_reg_reset(&IO_pin_handle_3, DIR);
    IO_pin_handle_reg_reset(&IO_pin_handle_3, OUT);
    IO_pin_handle_reg_set(&IO_pin_handle_3, REN);
    IO_pin_handle_reg_set(&IO_pin_handle_3, IES);

    // pin P6.3 - direction out, output level low
    IO_port_handle_register(&IO_port_driver_3, &IO_pin_handle_4, PIN_3);
    IO_pin_handle_reg_reset(&IO_pin_handle_4, OUT);
    IO_pin_handle_reg_set(&IO_pin_handle_4, DIR);

    // port 7 driver
    IO_port_driver_create(&IO_port_driver_4, PORT_7);

    // pin P7.1 direction in, pull-down resistor enable, interrupt edge high-to-low
    IO_port_handle_register(&IO_port_driver_4, &IO_pin_handle_5, PIN_1);
    IO_pin_handle_reg_reset(&IO_pin_handle_5, DIR);
    IO_pin_handle_reg_reset(&IO_pin_handle_5, OUT);
    IO_pin_handle_reg_set(&IO_pin_handle_5, REN);
    IO_pin_handle_reg_set(&IO_pin_handle_5, IES);

    // reset LOCKLPM5
    IO_debug_unlock();

    // pin P5.2 clear interrupt flag, set interrupt handler and interrupt enable
    vector_clear_interrupt_flag(&IO_pin_handle_2);
    vector_register_handler(&IO_pin_handle_2, test_interrupt_handler_P5_2, &IO_pin_handle_2, NULL);
    vector_set_enabled(&IO_pin_handle_2, true);

    // pin P6.1 clear interrupt flag, set interrupt handler and interrupt enable
    vector_clear_interrupt_flag(&IO_pin_handle_3);
    vector_register_handler(&IO_pin_handle_3, test_interrupt_handler_P6_1, &IO_pin_handle_3, NULL);
    vector_set_enabled(&IO_pin_handle_3, true);

    // pin P7.1 clear interrupt flag, set interrupt handler and interrupt enable
    vector_clear_interrupt_flag(&IO_pin_handle_5);
    vector_register_handler(&IO_pin_handle_5, test_interrupt_handler_P7_1, &IO_pin_handle_5, NULL);
    vector_set_enabled(&IO_pin_handle_5, true);

    interrupt_enable();

    assert(interrupt_handler_P5_2_call_count == 0);
    assert(interrupt_handler_P6_1_call_count == 0);
    assert(interrupt_handler_P7_1_call_count == 0);

    // --- trigger P5.2 interrupt (low-to-high) ---
    IO_pin_handle_reg_set(&IO_pin_handle_1, OUT);

    __delay_cycles(6);

    assert(interrupt_handler_P5_2_call_count == 1);
    assert(interrupt_handler_P6_1_call_count == 0);

    // --- trigger P6.1 interrupt (high-to-low) ---
    IO_pin_handle_reg_reset(&IO_pin_handle_1, OUT);

    __delay_cycles(6);

    assert(interrupt_handler_P5_2_call_count == 1);
    assert(interrupt_handler_P6_1_call_count == 1);

    // --- trigger P5.2 interrupt (low-to-high) ---
    IO_pin_handle_reg_set(&IO_pin_handle_1, OUT);

    __delay_cycles(6);

    assert(interrupt_handler_P5_2_call_count == 2);
    assert(interrupt_handler_P6_1_call_count == 1);

    // --- trigger P7.1 interrupt (high-to-low) ---
    IO_pin_handle_reg_set(&IO_pin_handle_4, OUT);

    __delay_cycles(6);

    assert(interrupt_handler_P7_1_call_count == 0);

    IO_pin_handle_reg_reset(&IO_pin_handle_4, OUT);

    __delay_cycles(6);

    assert(interrupt_handler_P7_1_call_count == 1);

    // --- test handle dispose ---
    dispose(&IO_pin_handle_3);

    IO_pin_handle_reg_reset(&IO_pin_handle_1, OUT);

    __delay_cycles(6);

    assert(interrupt_handler_P5_2_call_count == 2);
    assert(interrupt_handler_P6_1_call_count == 1);

    IO_pin_handle_reg_set(&IO_pin_handle_1, OUT);

    __delay_cycles(6);

    assert(interrupt_handler_P5_2_call_count == 3);
    assert(interrupt_handler_P6_1_call_count == 1);

    IO_pin_handle_reg_reset(&IO_pin_handle_1, OUT);

    __delay_cycles(6);

    assert(interrupt_handler_P5_2_call_count == 3);
    assert(interrupt_handler_P6_1_call_count == 1);

    // --- test driver dispose ---
    dispose(&IO_port_driver_2);

    IO_pin_handle_reg_set(&IO_pin_handle_1, OUT);

    __delay_cycles(6);

    assert(interrupt_handler_P5_2_call_count == 3);
    assert(interrupt_handler_P6_1_call_count == 1);

    IO_pin_handle_reg_reset(&IO_pin_handle_1, OUT);

    __delay_cycles(6);

    assert(interrupt_handler_P5_2_call_count == 3);
    assert(interrupt_handler_P6_1_call_count == 1);

    dispose(&IO_port_driver_1);
    dispose(&IO_port_driver_2);
    dispose(&IO_port_driver_3);
    dispose(&IO_port_driver_4);
}

// -------------------------------------------------------------------------------------

static void test_interrupt_handler_P5_2(IO_pin_handle_t *handle, uint8_t interrupt_pin) {
    assert(handle == &IO_pin_handle_2);
    assert(interrupt_pin == PIN_2);

    interrupt_handler_P5_2_call_count++;
}

static void test_interrupt_handler_P6_1(IO_pin_handle_t *handle, uint8_t interrupt_pin) {
    assert(handle == &IO_pin_handle_3);
    assert(interrupt_pin == PIN_1);

    interrupt_handler_P6_1_call_count++;
}

static void test_interrupt_handler_P7_1(IO_pin_handle_t *handle, uint8_t interrupt_pin) {
    assert(handle == &IO_pin_handle_5);
    assert(interrupt_pin == PIN_1);

    interrupt_handler_P7_1_call_count++;
}
