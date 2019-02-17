// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/queue/fifo.h>
#include <action.h>
#include <action/queue.h>

// -------------------------------------------------------------------------------------

#define __TEST_SIGNAL__   signal(0xA005)

static bool test_action_handler(action_arg_t, action_arg_t);
static dispose_function_t test_dispose_hook(Action_t *);
static void test_action_released_hook(Action_t *action, Action_queue_t *origin);

static volatile uint16_t test_dispose_hook_call_count;
static volatile uint16_t test_action_released_hook_call_count;
static volatile Action_t *expected_to_remove;
static volatile bool disable_expected_to_remove_check;

// -------------------------------------------------------------------------------------

void test_kernel_queue_fifo() {

    WDT_disable();
    default_clock_setup(0, DCOFSEL_0, DIVM__1);

    test_dispose_hook_call_count = test_action_released_hook_call_count = 0;
    disable_expected_to_remove_check = false;

    action_queue_create(&action_queue_1, false, false);

    action_create(&action_1, (dispose_function_t) test_dispose_hook, action_default_trigger, test_action_handler);
    action_create(&action_2, (dispose_function_t) test_dispose_hook, action_default_trigger, test_action_handler);
    action_create(&action_3, (dispose_function_t) test_dispose_hook, action_default_trigger, test_action_handler);
    action_create(&action_4, (dispose_function_t) test_dispose_hook, action_default_trigger, test_action_handler);

    action_on_released(&action_1) = test_action_released_hook;
    action_on_released(&action_2) = test_action_released_hook;
    action_on_released(&action_3) = test_action_released_hook;
    action_on_released(&action_4) = test_action_released_hook;

    sorted_set_item_priority(&action_1) = 1;
    sorted_set_item_priority(&action_2) = 3;
    sorted_set_item_priority(&action_3) = 2;
    sorted_set_item_priority(&action_4) = 4;

    // --- test fifo behavior ---
    action_queue_insert(&action_queue_1, &action_1);
    action_queue_insert(&action_queue_1, &action_2);
    action_queue_insert(&action_queue_1, &action_3);

    expected_to_remove = &action_1;

    assert(action_queue_pop(&action_queue_1) == &action_1);

    expected_to_remove = &action_2;

    assert(action_queue_pop(&action_queue_1) == &action_2);

    action_queue_insert(&action_queue_1, &action_4);

    expected_to_remove = &action_3;

    assert(action_queue_pop(&action_queue_1) == &action_3);

    expected_to_remove = &action_4;

    assert(action_queue_pop(&action_queue_1) == &action_4);

    assert(test_dispose_hook_call_count == 0);
    assert(test_action_released_hook_call_count == 4);

    assert(action_queue_is_empty(&action_queue_1));

    assert(action_queue_pop(&action_queue_1) == NULL);

    test_dispose_hook_call_count = test_action_released_hook_call_count = 0;

    // --- test dispose and on released hook on dispose ---
    action_queue_insert(&action_queue_1, &action_1);
    action_queue_insert(&action_queue_1, &action_2);
    action_queue_insert(&action_queue_1, &action_3);
    action_queue_insert(&action_queue_1, &action_4);

    expected_to_remove = &action_2;

    dispose(&action_2);

    expected_to_remove = &action_1;

    assert(action_queue_pop(&action_queue_1) == &action_1);

    expected_to_remove = &action_3;

    assert(action_queue_pop(&action_queue_1) == &action_3);

    expected_to_remove = NULL;

    dispose(&action_3);

    expected_to_remove = &action_4;

    assert(action_queue_pop(&action_queue_1) == &action_4);

    assert(action_queue_pop(&action_queue_1) == NULL);

    assert(test_dispose_hook_call_count == 2);
    assert(test_action_released_hook_call_count == 4);

    dispose(&action_1);
    dispose(&action_4);

    test_dispose_hook_call_count = test_action_released_hook_call_count = 0;

    // --- test close ---
    disable_expected_to_remove_check = true;

    action_create(&action_1, (dispose_function_t) test_dispose_hook, action_default_trigger, test_action_handler);
    action_create(&action_2, (dispose_function_t) test_dispose_hook, action_default_trigger, test_action_handler);
    action_create(&action_3, (dispose_function_t) test_dispose_hook, action_default_trigger, test_action_handler);
    action_create(&action_4, (dispose_function_t) test_dispose_hook, action_default_trigger, test_action_handler);

    action_on_released(&action_1) = test_action_released_hook;
    action_on_released(&action_2) = test_action_released_hook;
    action_on_released(&action_3) = test_action_released_hook;
    action_on_released(&action_4) = test_action_released_hook;

    action_queue_insert(&action_queue_1, &action_1);
    action_queue_insert(&action_queue_1, &action_2);
    action_queue_insert(&action_queue_1, &action_3);
    action_queue_insert(&action_queue_1, &action_4);

    action_queue_close(&action_queue_1, __TEST_SIGNAL__);

    assert(test_dispose_hook_call_count == 0);
    assert(test_action_released_hook_call_count == 4);
}

static bool test_action_handler(action_arg_t _, signal_t signal) {
    assert(signal == __TEST_SIGNAL__);

    // remove action from queue
    return false;
}

static dispose_function_t test_dispose_hook(Action_t *_) {

    test_dispose_hook_call_count++;

    return NULL;
}

static void test_action_released_hook(Action_t *action, Action_queue_t *origin) {
    assert(origin == &action_queue_1);

    if ( ! disable_expected_to_remove_check) {
        assert(action == expected_to_remove);
    }

    test_action_released_hook_call_count++;
}
