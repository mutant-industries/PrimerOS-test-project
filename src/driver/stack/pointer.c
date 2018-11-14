// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/stack/pointer.h>
#include <driver/stack.h>
#include <driver/cpu.h>
#include <compiler.h>

// -------------------------------------------------------------------------------------

static volatile void *test_set, *test_get, *backup;
volatile void *__SP__;

#define read_SP() \
    __asm__("   "__mov__" SP, &__SP__");

// -------------------------------------------------------------------------------------

void test_driver_stack_pointer() {

    WDT_disable();
    default_clock_setup();

    read_SP();

    // --- test 16-bit read, backup shall be restored when test finished ---
    stack_pointer_get(&backup);

    if (backup != __SP__) {
        test_fail();
    }

    // --- test 16-bit read / write ---
    test_set = (void *) 0xAAAA;

    stack_pointer_set(test_set);

    read_SP();

    stack_pointer_get(&test_get);

    if (test_set != __SP__) {
        test_fail();
    }

    if (test_get != __SP__) {
        test_fail();
    }

#if defined(_DATA_MODEL_LARGE_) && ! (defined(_TI_COMPILER_) && defined(__STACK_POINTER_20_BIT_SUPPORT_DISABLE__))
    // --- test 20-bit read / write ---
    test_set = (void *) 0x1AAAA;

    stack_pointer_set(test_set);

    read_SP();

    stack_pointer_get(&test_get);

    if (test_set != __SP__) {
        test_fail();
    }

    if (test_get != __SP__) {
        test_fail();
    }

#endif

    stack_pointer_set(backup);
}
