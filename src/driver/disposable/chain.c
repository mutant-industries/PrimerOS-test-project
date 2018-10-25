// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/disposable/chain.h>
#include <driver/disposable.h>
#include <driver/vector.h>
#include <driver/wdt.h>
#include <process.h>

// -------------------------------------------------------------------------------------

static dispose_function_t _test_dispose_hook_1(Disposable_t *disposable);
static dispose_function_t _test_dispose_hook_2(Disposable_t *disposable);
static void _test_ISR(void);

Vector_handle_t h;
Process_control_block_t pcb;
bool dispose_hook_1_called;
bool dispose_hook_2_called;

// -------------------------------------------------------------------------------------

void test_driver_disposable_chain() {
    uint16_t vector_content_backup;

    dispose_hook_1_called = dispose_hook_2_called = false;
    running_process = &pcb;

    __WDT_hold();
    default_clock_setup();

    vector_handle_register(&h, (dispose_function_t) _test_dispose_hook_1, WDT_VECTOR,
                           (uint16_t) &SFRIE1, WDTIE, (uint16_t) &SFRIFG1, WDTIFG);

    vector_content_backup = __vector(WDT_VECTOR);

    h.register_handler(&h, (void (*)(void)) _test_ISR, true);

    if (__vector(WDT_VECTOR) == vector_content_backup) {
        test_fail();
    }

    dispose(&h);

    if ( ! dispose_hook_1_called) {
        test_fail();
    }

    if ( ! dispose_hook_2_called) {
        test_fail();
    }

    if (__vector(WDT_VECTOR) != vector_content_backup) {
        test_fail();
    }
}

static dispose_function_t _test_dispose_hook_1(Disposable_t *disposable) {

    dispose_hook_1_called = true;

    if (disposable != (Disposable_t *) &h) {
        test_fail();
    }

    return (dispose_function_t) _test_dispose_hook_2;
}

static dispose_function_t _test_dispose_hook_2(Disposable_t *disposable) {

    dispose_hook_2_called = true;

    if (disposable != (Disposable_t *) &h) {
        test_fail();
    }

    return NULL;
}

// -------------------------------------------------------------------------------------

static __interrupt void _test_ISR() {
    // just for reference
}
