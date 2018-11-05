// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/disposable/resource.h>
#include <driver/disposable.h>
#include <process.h>

// -------------------------------------------------------------------------------------

static dispose_function_t test_dispose_hook(Disposable_t *);

static Disposable_t r1, r2, r3, r4, r5;
static Process_control_block_t pcb;
static bool dispose_hook_called;

// -------------------------------------------------------------------------------------

void test_driver_disposable_resource() {

    Disposable_t *disposable;
    uint8_t resource_list_length, resource_list_expected_length;

#ifdef __RESOURCE_MANAGEMENT_ENABLE__
    running_process = &pcb;

    WDT_disable();
    default_clock_setup();

    __dispose_hook_register(&r1, test_dispose_hook);
    __dispose_hook_register(&r2, test_dispose_hook);
    __dispose_hook_register(&r3, test_dispose_hook);
    __dispose_hook_register(&r4, test_dispose_hook);
    __dispose_hook_register(&r5, test_dispose_hook);

    disposable = pcb._resource_list;

    if (disposable != &r5) {
        test_fail();
    }

    if ((disposable = disposable->_next) != &r4) {
        test_fail();
    }

    if ((disposable = disposable->_next) != &r3) {
        test_fail();
    }

    if ((disposable = disposable->_next) != &r2) {
        test_fail();
    }

    if ((disposable = disposable->_next) != &r1) {
        test_fail();
    }

    // --- dispose list head ---
    dispose_hook_called = false;

    dispose(&r5);

    if ( ! dispose_hook_called) {
        test_fail();
    }

    for (disposable = pcb._resource_list, resource_list_length = 0; disposable; disposable = disposable->_next, resource_list_length++) {
        if (disposable == &r5) {
            test_fail();
        }
    }

    if (resource_list_length != 4) {
        test_fail();
    }

    // --- dispose resource, that is not on owner list ---
    dispose_hook_called = false;

    dispose(&r5);

    if (dispose_hook_called) {
        test_fail();
    }

    // --- dispose list tail ---
    dispose_hook_called = false;

    dispose(&r1);

    if ( ! dispose_hook_called) {
        test_fail();
    }

    for (disposable = pcb._resource_list, resource_list_length = 0; disposable; disposable = disposable->_next, resource_list_length++) {
        if (disposable == &r1) {
            test_fail();
        }
    }

    if (resource_list_length != 3) {
        test_fail();
    }

    // --- dispose resource, that is not on owner list ---
    dispose_hook_called = false;

    dispose(&r1);

    if (dispose_hook_called) {
        test_fail();
    }
    // --- dispose list middle resource ---
    dispose_hook_called = false;

    dispose(&r3);

    if ( ! dispose_hook_called) {
        test_fail();
    }

    for (disposable = pcb._resource_list, resource_list_length = 0; disposable; disposable = disposable->_next, resource_list_length++) {
        if (disposable == &r3) {
            test_fail();
        }
    }

    if (resource_list_length != 2) {
        test_fail();
    }

    // --- dispose resource, that is not on owner list ---
    dispose_hook_called = false;

    dispose(&r3);

    if (dispose_hook_called) {
        test_fail();
    }

    // --- dispose remaining resources ---
    resource_list_expected_length = 2;

    while ((disposable = pcb._resource_list)) {
        dispose_hook_called = false;

        dispose(disposable);

        resource_list_expected_length--;

        if ( ! dispose_hook_called) {
            test_fail();
        }

        dispose_hook_called = false;

        dispose(disposable);

        if (dispose_hook_called) {
            test_fail();
        }

        for (disposable = pcb._resource_list, resource_list_length = 0; disposable;
             disposable = disposable->_next, resource_list_length++);


        if (resource_list_expected_length != resource_list_length) {
            test_fail();
        }
    }

    if (resource_list_expected_length) {
        test_fail();
    }

#endif /* __RESOURCE_MANAGEMENT_ENABLE__ */
}

static dispose_function_t test_dispose_hook(Disposable_t *_) {

    dispose_hook_called = true;

    return NULL;
}
