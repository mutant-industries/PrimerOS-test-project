// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/vector/trigger.h>
#include <driver/disposable.h>
#include <driver/vector.h>
#include <process.h>

// -------------------------------------------------------------------------------------

static void test_handler(Vector_handle_t *);

static uint8_t interrupt_handler_call_count;

// -------------------------------------------------------------------------------------

void test_driver_vector_trigger() {
    uint16_t vector_content_backup;

    interrupt_handler_call_count = 0;

    WDT_disable();
    default_clock_setup(0, DCOFSEL_0, DIVM__1);

    vector_handle_register(&vector, NULL, COMP_E_VECTOR, (uint16_t) &CEINT, CERDYIE, (uint16_t) &CEINT, CERDYIFG);

    vector_content_backup = __vector(COMP_E_VECTOR);

    __enable_interrupt();

    assert(vector_register_handler(&vector, test_handler, &vector, NULL));

    assert_not(vector_content_backup == __vector(COMP_E_VECTOR));
    assert( ! interrupt_handler_call_count);

    // --- trigger enabled ---
    vector_set_enabled(&vector, true);

    assert( ! vector_trigger(&vector));

    assert(interrupt_handler_call_count == 1);

    assert( ! vector_trigger(&vector));

    assert(interrupt_handler_call_count == 2);

    interrupt_handler_call_count = 0;

    // --- trigger disabled ---
    vector_set_enabled(&vector, false);

    assert( ! vector_trigger(&vector));

    assert( ! interrupt_handler_call_count);

    assert( ! vector_trigger(&vector));

    assert( ! interrupt_handler_call_count);

    // --- should trigger on enable ---
    vector_set_enabled(&vector, true);

    assert(interrupt_handler_call_count == 1);

    assert( ! vector_trigger(&vector));

    assert(interrupt_handler_call_count == 2);

    interrupt_handler_call_count = 0;

    // --- dispose test ---
    dispose(&vector);

    assert(vector_trigger(&vector));

    assert( ! interrupt_handler_call_count);

    assert(vector_content_backup == __vector(COMP_E_VECTOR));
}

void test_handler(Vector_handle_t *handle) {

    assert(handle == &vector);

    interrupt_handler_call_count++;

    vector_clear_interrupt_flag(handle);
}
