// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/dispose/chain.h>
#include <driver/disposable.h>
#include <driver/vector.h>
#include <process.h>

// -------------------------------------------------------------------------------------

void driver_disposable_chain_test_ISR(void);
static dispose_function_t test_dispose_hook_1(Disposable_t *disposable);
static dispose_function_t test_dispose_hook_2(Disposable_t *disposable);

static bool dispose_hook_1_called;
static bool dispose_hook_2_called;

// -------------------------------------------------------------------------------------

void test_kernel_dispose_chain() {
    uint16_t vector_content_backup;

    dispose_hook_1_called = dispose_hook_2_called = false;

    WDT_disable();
    default_clock_setup(0, DCOFSEL_0, DIVM__1);

    vector_handle_register(&vector, (dispose_function_t) test_dispose_hook_1, WDT_VECTOR,
                           (uint16_t) &SFRIE1, WDTIE, (uint16_t) &SFRIFG1, WDTIFG);

    vector_content_backup = __vector(WDT_VECTOR);

    vector_register_raw_handler(&vector, driver_disposable_chain_test_ISR, true);

    assert_not(__vector(WDT_VECTOR) == vector_content_backup);

    dispose(&vector);

    assert(dispose_hook_1_called);
    assert(dispose_hook_2_called);
    assert(__vector(WDT_VECTOR) == vector_content_backup);
}

static dispose_function_t test_dispose_hook_1(Disposable_t *disposable) {

    dispose_hook_1_called = true;

    assert(disposable == (Disposable_t *) &vector);

    return (dispose_function_t) test_dispose_hook_2;
}

static dispose_function_t test_dispose_hook_2(Disposable_t *disposable) {

    dispose_hook_2_called = true;

    assert(disposable == (Disposable_t *) &vector);

    return NULL;
}

// -------------------------------------------------------------------------------------

void __interrupt driver_disposable_chain_test_ISR() {
    // just for reference
}
