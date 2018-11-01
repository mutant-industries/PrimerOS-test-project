// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/vector/trigger.h>
#include <driver/disposable.h>
#include <driver/vector.h>
#include <driver/wdt.h>
#include <process.h>

// -------------------------------------------------------------------------------------

static void test_handler(Vector_handle_t *);

static Vector_handle_t h;
static Process_control_block_t pcb;
static uint8_t interrupt_handler_call_count;

// -------------------------------------------------------------------------------------

void test_driver_vector_trigger() {
    uint16_t vector_content_backup;

    interrupt_handler_call_count = 0;
    running_process = &pcb;

    WDT_hold();
    default_clock_setup();

    vector_handle_register(&h, NULL, COMP_E_VECTOR, (uint16_t) &CEINT, CERDYIE, (uint16_t) &CEINT, CERDYIFG);

    vector_content_backup = __vector(COMP_E_VECTOR);

    __enable_interrupt();

    if (h.register_handler(&h, (void (*)(void *)) test_handler, &h) == NULL) {
        test_fail();
    }

    if (vector_content_backup == __vector(COMP_E_VECTOR)) {
        test_fail();
    }

    if (interrupt_handler_call_count) {
        test_fail();
    }

    // --- trigger enabled ---
    h.set_enabled(&h, true);

    if (h.trigger(&h)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 1) {
        test_fail();
    }

    if (h.trigger(&h)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 2) {
        test_fail();
    }

    interrupt_handler_call_count = 0;

    // --- trigger disabled ---
    h.set_enabled(&h, false);

    if (h.trigger(&h)) {
        test_fail();
    }

    if (interrupt_handler_call_count) {
        test_fail();
    }

    if (h.trigger(&h)) {
        test_fail();
    }

    if (interrupt_handler_call_count) {
        test_fail();
    }

    // --- should trigger on enable ---
    h.set_enabled(&h, true);

    if (interrupt_handler_call_count != 1) {
        test_fail();
    }

    if (h.trigger(&h)) {
        test_fail();
    }

    if (interrupt_handler_call_count != 2) {
        test_fail();
    }

    interrupt_handler_call_count = 0;

    // --- dispose test ---
    dispose(&h);

    if ( ! h.trigger(&h)) {
        test_fail();
    }

    if (interrupt_handler_call_count) {
        test_fail();
    }

    if (vector_content_backup != __vector(COMP_E_VECTOR)) {
        test_fail();
    }
}

void test_handler(Vector_handle_t *handle) {

    if (handle != &h) {
        test_fail();
    }

    interrupt_handler_call_count++;

    handle->clear_interrupt_flag(handle);
}
