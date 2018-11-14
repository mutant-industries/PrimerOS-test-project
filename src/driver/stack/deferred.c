// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/stack/deferred.h>
#include <driver/stack.h>
#include <driver/cpu.h>

// -------------------------------------------------------------------------------------

__asm__(" .global return_to_label");

extern void return_to_label(void);
static void param_test(void *);
static void *return_value_test(void);

#ifdef _CODE_MODEL_LARGE_
#define ret \
    __asm__("   RETA");
#else
#define ret \
    __asm__("   RET");
#endif

// -------------------------------------------------------------------------------------

#define _TEST_STACK_SIZE_   0x3E
uint8_t test_stack[_TEST_STACK_SIZE_] __attribute__ ((aligned (16)));

static volatile void *stack_pointer, *backup;
static volatile bool param_test_called;

#ifdef _DATA_MODEL_LARGE_
void *test_param = (void *) 0x1A5A5;
void *test_return_value = (void *) 0x15A5A;
#else
void *test_param = (void *) 0xB4B4;
void *test_return_value = (void *) 0x4B4B;
#endif

// -------------------------------------------------------------------------------------

void test_driver_stack_deferred() {
    void *test_return_value_result;

    param_test_called = false;

    WDT_disable();
    default_clock_setup();

    stack_save_context(&backup);

    // --- test deferred stack context init ---
    deferred_stack_pointer_init(&stack_pointer, test_stack, _TEST_STACK_SIZE_);
    deferred_stack_push_return_address(&stack_pointer, return_to_label);
    deferred_stack_context_init(&stack_pointer, param_test, test_param);

    stack_restore_context(&stack_pointer);

    reti;
__asm__("return_to_label:");

    if ( ! param_test_called) {
        test_fail();
    }

    // --- test deferred stack return value set ---
    test_return_value_result = return_value_test();

    if (test_return_value_result != test_return_value) {
        test_fail();
    }

    stack_restore_context(&backup);
}

static void param_test(void *param) {
    if (param != test_param) {
        test_fail();
    }

    param_test_called = true;
}

static __naked void *return_value_test() {
    stack_save_context(&stack_pointer);

    deferred_stack_store_return_value(stack_pointer, test_return_value);

    stack_restore_context(&stack_pointer);

    ret;
}
