// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/dispose/resource.h>
#include <driver/disposable.h>
#include <process.h>

// -------------------------------------------------------------------------------------

static dispose_function_t test_dispose_hook(Disposable_t *);

static bool dispose_hook_called;

// -------------------------------------------------------------------------------------

void test_kernel_dispose_resource() {

    Disposable_t *disposable;
    uint8_t resource_list_length, resource_list_expected_length;

#ifdef __RESOURCE_MANAGEMENT_ENABLE__
    running_process = &init_process;

    WDT_disable();
    default_clock_setup(0, DCOFSEL_0, DIVM__1);

    init_process._resource_list = NULL;

    __dispose_hook_register(&action_1, test_dispose_hook);
    __dispose_hook_register(&action_2, test_dispose_hook);
    __dispose_hook_register(&action_3, test_dispose_hook);
    __dispose_hook_register(&action_4, test_dispose_hook);
    __dispose_hook_register(&action_5, test_dispose_hook);

    disposable = init_process._resource_list;

    assert(disposable == (Disposable_t *) &action_5);
    assert((disposable = disposable->_next) == (Disposable_t *) &action_4);
    assert((disposable = disposable->_next) == (Disposable_t *) &action_3);
    assert((disposable = disposable->_next) == (Disposable_t *) &action_2);
    assert((disposable = disposable->_next) == (Disposable_t *) &action_1);

    // --- dispose list head ---
    dispose_hook_called = false;

    dispose(&action_5);

    assert(dispose_hook_called);

    for (disposable = init_process._resource_list, resource_list_length = 0; disposable;
                disposable = disposable->_next, resource_list_length++) {

        assert_not(disposable == (Disposable_t *) &action_5);
    }

    assert(resource_list_length == 4);

    // --- dispose resource, that is not on owner list ---
    dispose_hook_called = false;

    dispose(&action_5);

    assert_not(dispose_hook_called);

    // --- dispose list tail ---
    dispose_hook_called = false;

    dispose(&action_1);

    assert(dispose_hook_called);

    for (disposable = init_process._resource_list, resource_list_length = 0; disposable;
                disposable = disposable->_next, resource_list_length++) {

        assert_not(disposable == (Disposable_t *) &action_1);
    }

    assert(resource_list_length == 3);

    // --- dispose resource, that is not on owner list ---
    dispose_hook_called = false;

    dispose(&action_1);

    assert_not(dispose_hook_called);

    // --- dispose list middle resource ---
    dispose_hook_called = false;

    dispose(&action_3);

    assert(dispose_hook_called);

    for (disposable = init_process._resource_list, resource_list_length = 0; disposable;
                disposable = disposable->_next, resource_list_length++) {

        assert_not(disposable == (Disposable_t *) &action_3);
    }

    assert(resource_list_length == 2);

    // --- dispose resource, that is not on owner list ---
    dispose_hook_called = false;

    dispose(&action_3);

    assert_not(dispose_hook_called);

    // --- dispose remaining resources ---
    resource_list_expected_length = 2;

    while ((disposable = init_process._resource_list)) {
        dispose_hook_called = false;

        dispose(disposable);

        resource_list_expected_length--;

        assert(dispose_hook_called);

        dispose_hook_called = false;

        dispose(disposable);

        assert_not(dispose_hook_called);

        for (disposable = init_process._resource_list, resource_list_length = 0; disposable;
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
