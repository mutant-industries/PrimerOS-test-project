// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/action/sorting.h>
#include <action.h>
#include <action/queue.h>
#include <process.h>

// -------------------------------------------------------------------------------------

#define __TEST_SIGNAL__   signal(0xA5A5)

static void test_action_executor(Action_t *, signal_t signal);
static bool test_action_handler(action_arg_t, action_arg_t);
static dispose_function_t test_dispose_hook(Action_t *);

static void test_queue(Action_queue_t *queue, uint16_t expected_size, Action_t *expected_in_queue, uint16_t expected_at_index,
                            Action_t *expected_not_in_queue);

static volatile uint16_t test_action_executor_call_count;
static volatile uint16_t test_action_handler_call_count;
static volatile uint16_t test_dispose_hook_call_count;

static volatile bool handler_next_return_value;

// -------------------------------------------------------------------------------------

void test_kernel_action_sorting() {

    test_action_executor_call_count = test_dispose_hook_call_count = 0;
    action_queue_init(&action_queue_1, true);

    WDT_disable();
    default_clock_setup();

    action_register(&action_1, (dispose_function_t) test_dispose_hook, test_action_executor, NULL, NULL, NULL);
    action_register(&action_2, (dispose_function_t) test_dispose_hook, test_action_executor, NULL, NULL, NULL);
    action_register(&action_3, (dispose_function_t) test_dispose_hook, test_action_executor, NULL, NULL, NULL);
    action_register(&action_4, (dispose_function_t) test_dispose_hook, test_action_executor, NULL, NULL, NULL);
    action_register(&action_5, (dispose_function_t) test_dispose_hook, test_action_executor, NULL, NULL, NULL);

    sorted_queue_item_priority(&action_1) = 10;
    sorted_queue_item_priority(&action_2) = 10;
    sorted_queue_item_priority(&action_3) = 20;
    sorted_queue_item_priority(&action_4) = 20;
    sorted_queue_item_priority(&action_5) = 30;

    // --- test empty ---
    test_queue(&action_queue_1, 0, NULL, NULL, NULL);

    action_queue_trigger_all(&action_queue_1, __TEST_SIGNAL__);

    assert( ! test_action_executor_call_count);

    // --- test sorting ---
    assert(enqueue(&action_3, &action_queue_1));

    test_queue(&action_queue_1, 1, &action_3, 0, NULL);

    assert_not(enqueue(&action_4, &action_queue_1));

    test_queue(&action_queue_1, 2, &action_4, 1, NULL);

    assert_not(enqueue(&action_1, &action_queue_1));

    test_queue(&action_queue_1, 3, &action_1, 2, NULL);

    assert_not(enqueue(&action_2, &action_queue_1));

    test_queue(&action_queue_1, 4, &action_2, 3, NULL);

    assert(enqueue(&action_5, &action_queue_1));

    test_queue(&action_queue_1, 5, &action_5, 0, NULL);

    // --- trigger all ---
    action_queue_trigger_all(&action_queue_1, __TEST_SIGNAL__);

    assert(test_action_executor_call_count == 5);

    test_action_executor_call_count = 0;

    // --- test removal ---
    action_remove(&action_1);

    test_queue(&action_queue_1, 4, NULL, NULL, &action_1);

    assert( ! test_dispose_hook_call_count);

    action_queue_trigger_all(&action_queue_1, __TEST_SIGNAL__);

    assert(test_action_executor_call_count == 4);

    test_action_executor_call_count = 0;

    // --- test dispose ---
    dispose(&action_4);

    test_queue(&action_queue_1, 3, NULL, NULL, &action_4);

    assert(test_dispose_hook_call_count == 1);

    test_dispose_hook_call_count = 0;

    action_queue_trigger_all(&action_queue_1, __TEST_SIGNAL__);

    assert(test_action_executor_call_count == 3);

    test_action_executor_call_count = 0;

    // --- test remove head ---
    action_remove(&action_5);

    test_queue(&action_queue_1, 2, NULL, NULL, &action_5);

    // --- test remove tail ---
    action_remove(&action_2);

    test_queue(&action_queue_1, 1, NULL, NULL, &action_2);

    // --- test remove not queued ---
    action_remove(&action_2);

    test_queue(&action_queue_1, 1, &action_3, 0, NULL);

    // --- test remove last ---
    dispose(&action_3);

    test_queue(&action_queue_1, 0, NULL, NULL, NULL);

    // ------------------------

    dispose(&action_1);
    dispose(&action_2);
    dispose(&action_3);
    dispose(&action_4);
    dispose(&action_5);

    test_dispose_hook_call_count = test_action_handler_call_count = 0;

    action_register(&action_1, (dispose_function_t) test_dispose_hook, action_default_executor, test_action_handler, NULL, NULL);
    action_register(&action_2, (dispose_function_t) test_dispose_hook, action_default_executor, test_action_handler, NULL, NULL);
    action_register(&action_3, (dispose_function_t) test_dispose_hook, action_default_executor, test_action_handler, NULL, NULL);
    action_register(&action_4, (dispose_function_t) test_dispose_hook, action_default_executor, test_action_handler, NULL, NULL);

    assert(enqueue(&action_1, &action_queue_1));

    assert_not(enqueue(&action_2, &action_queue_1));

    assert(enqueue(&action_3, &action_queue_1));

    assert_not(enqueue(&action_4, &action_queue_1));

    test_queue(&action_queue_1, 4, NULL, NULL, NULL);

    // --- test default executor - queue removal on handler return true (remove even), test action_execute_all() ---
    handler_next_return_value = true;

    action_queue_trigger_all(&action_queue_1, __TEST_SIGNAL__);

    assert(test_action_handler_call_count == 4);
    assert( ! test_dispose_hook_call_count);

    test_queue(&action_queue_1, 2, &action_2, 1, &action_1);
    test_queue(&action_queue_1, 2, &action_4, 0, &action_3);

    test_action_handler_call_count = 0;

    // --- test default executor - queue removal on handler return true (remove odd), test action_execute_all() ---
    handler_next_return_value = false;

    action_queue_trigger_all(&action_queue_1, __TEST_SIGNAL__);

    assert(test_action_handler_call_count == 2);
    assert( ! test_dispose_hook_call_count);

    test_queue(&action_queue_1, 1, &action_4, 0, &action_2);

    test_action_handler_call_count = 0;

    // --- test action priority change - action not in queue ---
    assert_not(action_set_priority(&action_3, 100));

    assert(sorted_queue_item_priority(&action_3) == 100);

    test_queue(&action_queue_1, 1, &action_4, 0, &action_3);

    // --- test action priority change - single action in queue ---
    assert(action_set_priority(&action_4, 40));

    assert(sorted_queue_item_priority(&action_4) == 40);

    test_queue(&action_queue_1, 1, &action_4, 0, NULL);

    // --- test action priority change - two actions in queue ---
    assert_not(enqueue(&action_2, &action_queue_1));

    test_queue(&action_queue_1, 2, &action_2, 1, NULL);

    assert(action_set_priority(&action_2, 60));

    test_queue(&action_queue_1, 2, &action_2, 0, NULL);

    assert_not(action_set_priority(&action_2, 20));

    test_queue(&action_queue_1, 2, &action_2, 1, NULL);

    // --- test action priority change - more actions in queue ---
    assert(enqueue(&action_3, &action_queue_1));

    test_queue(&action_queue_1, 3, &action_3, 0, NULL);

    assert(action_set_priority(&action_3, 60));

    test_queue(&action_queue_1, 3, &action_3, 0, NULL);

    assert_not(action_set_priority(&action_3, 30));

    test_queue(&action_queue_1, 3, &action_3, 1, NULL);

    assert_not(action_set_priority(&action_3, 25));

    test_queue(&action_queue_1, 3, &action_3, 1, NULL);

    assert_not(action_set_priority(&action_3, 15));

    test_queue(&action_queue_1, 3, &action_3, 2, NULL);

    assert_not(action_set_priority(&action_3, 0));

    test_queue(&action_queue_1, 3, &action_3, 2, NULL);

    assert(action_set_priority(&action_3, 100));

    test_queue(&action_queue_1, 3, &action_3, 0, NULL);

    assert_not(action_set_priority(&action_3, 0));

    test_queue(&action_queue_1, 3, &action_3, 2, NULL);

    assert_not(action_set_priority(&action_3, 15));

    test_queue(&action_queue_1, 3, &action_3, 2, NULL);

    assert_not(action_set_priority(&action_3, 25));

    test_queue(&action_queue_1, 3, &action_3, 1, NULL);

    assert_not(action_set_priority(&action_3, 30));

    test_queue(&action_queue_1, 3, &action_3, 1, NULL);

    assert(action_set_priority(&action_3, 60));

    test_queue(&action_queue_1, 3, &action_3, 0, NULL);

    // ------------------------

    dispose(&action_1);
    dispose(&action_2);
    dispose(&action_3);
    dispose(&action_4);
    dispose(&action_5);
}

static void test_queue(Action_queue_t *queue, uint16_t expected_size, Action_t *expected_in_queue, uint16_t expected_at_index,
                            Action_t *expected_not_in_queue) {

    Action_t *head, *tail, *current, *prev;
    bool expected_found;
    uint16_t last_priority_seen;
    uint16_t size;

    if (action_queue_empty(queue)) {

        assert(expected_size == 0);
        assert( ! expected_in_queue);

        return;
    }

    head = action_queue_head(queue);
    tail = action(deque_item_prev(head));
    current = head;
    prev = NULL;
    expected_found = false;
    last_priority_seen = sorted_queue_item_priority(current);
    size = 0;

    while (current) {
        // --- test chaining in reverse direction ---
        assert( ! prev || action(deque_item_prev(current)) == prev);

        // --- test chaining in reverse direction ---
        assert(sorted_queue_item_priority(current) <= last_priority_seen);

        // --- test not expected not in queue if set ---
        assert( ! expected_not_in_queue || current != expected_not_in_queue);

        // --- test expected in queue if set ---
        if (expected_in_queue && current == expected_in_queue) {
            expected_found = true;

            // --- test expected found at expected index ---
            assert(size == expected_at_index);
        }

        size++;

        if (current == tail) {
            break;
        }

        last_priority_seen = sorted_queue_item_priority(current);

        prev = current;
        current = action(deque_item_next(current));
    }

    // --- test tail reached ---
    assert(current == tail);

    // --- test head reachable from tail ---
    assert(action(deque_item_next(tail)) == head);

    // --- test expected size ---
    assert(size == expected_size);

    // --- test expected in queue if set ---
    assert_not(expected_in_queue && ! expected_found);
}

// -------------------------------------------------------------------------------------

static void test_action_executor(Action_t *_this, signal_t signal) {

    assert(signal == __TEST_SIGNAL__);

    test_action_executor_call_count++;
}

static bool test_action_handler(action_arg_t arg_1, action_arg_t arg_2) {
    bool current_return_value = handler_next_return_value;

    test_action_handler_call_count++;
    handler_next_return_value = ! handler_next_return_value;

    return current_return_value;
}

static dispose_function_t test_dispose_hook(Action_t *_) {

    test_dispose_hook_call_count++;

    return NULL;
}
