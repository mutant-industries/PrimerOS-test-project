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

    assert(disposable == &r5);
    assert((disposable = disposable->_next) == &r4);
    assert((disposable = disposable->_next) == &r3);
    assert((disposable = disposable->_next) == &r2);
    assert((disposable = disposable->_next) == &r1);

    // --- dispose list head ---
    dispose_hook_called = false;

    dispose(&r5);

    assert(dispose_hook_called);

    for (disposable = pcb._resource_list, resource_list_length = 0; disposable; disposable = disposable->_next, resource_list_length++) {
        assert_not(disposable == &r5);
    }

    assert(resource_list_length == 4);

    // --- dispose resource, that is not on owner list ---
    dispose_hook_called = false;

    dispose(&r5);

    assert_not(dispose_hook_called);

    // --- dispose list tail ---
    dispose_hook_called = false;

    dispose(&r1);

    assert(dispose_hook_called);

    for (disposable = pcb._resource_list, resource_list_length = 0; disposable; disposable = disposable->_next, resource_list_length++) {
        assert_not(disposable == &r1);
    }

    assert(resource_list_length == 3);

    // --- dispose resource, that is not on owner list ---
    dispose_hook_called = false;

    dispose(&r1);

    assert_not(dispose_hook_called);

    // --- dispose list middle resource ---
    dispose_hook_called = false;

    dispose(&r3);

    assert(dispose_hook_called);

    for (disposable = pcb._resource_list, resource_list_length = 0; disposable; disposable = disposable->_next, resource_list_length++) {
        assert_not(disposable == &r3);
    }

    assert(resource_list_length == 2);

    // --- dispose resource, that is not on owner list ---
    dispose_hook_called = false;

    dispose(&r3);

    assert_not(dispose_hook_called);

    // --- dispose remaining resources ---
    resource_list_expected_length = 2;

    while ((disposable = pcb._resource_list)) {
        dispose_hook_called = false;

        dispose(disposable);

        resource_list_expected_length--;

        assert(dispose_hook_called);

        dispose_hook_called = false;

        dispose(disposable);

        assert_not(dispose_hook_called);

        for (disposable = pcb._resource_list, resource_list_length = 0; disposable;
             disposable = disposable->_next, resource_list_length++);


        assert(resource_list_expected_length == resource_list_length);
    }

    assert( ! resource_list_expected_length);

#endif /* __RESOURCE_MANAGEMENT_ENABLE__ */
}

static dispose_function_t test_dispose_hook(Disposable_t *_) {

    dispose_hook_called = true;

    return NULL;
}
