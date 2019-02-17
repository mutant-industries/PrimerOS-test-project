// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/queue/priority.h>

// -------------------------------------------------------------------------------------

static signal_t test_process_1_entry_point();

static volatile bool process_1_executed;

// -------------------------------------------------------------------------------------

static void init() {
    // initialize empty test actions
    action_create(&action_1, NULL, NULL);
    action_create(&action_2, NULL, NULL);
    action_create(&action_3, NULL, NULL);
    action_create(&action_4, NULL, NULL);
    action_create(&action_5, NULL, NULL);

    // initialize sorted action queues with action inheriting their head priority
    action_queue_create(&action_queue_1, true, false, &action_1, action_default_set_priority);
    action_queue_create(&action_queue_2, true, false, &action_2, action_default_set_priority);
    action_queue_create(&action_queue_3, true, false, &action_3, action_default_set_priority);
    action_queue_create(&action_queue_4, true, false, &action_4, action_default_set_priority);

    // insert actions to queues so that action with lower index inherits priority of action with higher index
    action_queue_insert(&action_queue_1, &action_2);
    action_queue_insert(&action_queue_2, &action_3);
    action_queue_insert(&action_queue_3, &action_4);
    action_queue_insert(&action_queue_4, &action_5);

    // initialize test process
    zerofill(&process_1.create_config);

    process_1.create_config.priority = 1;
    process_1.create_config.stack_addr_low = (data_pointer_register_t) test_stack_1;
    process_1.create_config.stack_size = __TEST_STACK_SIZE__;
    process_1.create_config.entry_point = (process_entry_point_t) test_process_1_entry_point;

    // --- create process with higher priority than init process ---
    process_create(&process_1);
}

void test_kernel_queue_priority() {

    WDT_disable();
    default_clock_setup(0, DCOFSEL_0, DIVM__1);

    process_1_executed = false;

    // just for running process to be set, process local storage is required
    default_kernel_start(0, init, false);

    assert_not(process_1_executed);

    // --- start test ---
    process_schedule(&process_1, 0);

    assert(process_1_executed);
}

static signal_t test_process_1_entry_point() {

    process_current_local() = &process_1;

    // --- test no action was manipulated after init ---
    assert( ! sorted_set_item_priority(&action_1));
    assert( ! sorted_set_item_priority(&action_2));
    assert( ! sorted_set_item_priority(&action_3));
    assert( ! sorted_set_item_priority(&action_4));
    assert( ! sorted_set_item_priority(&action_5));

    // --- test chained priority inheritance (stack usage must be checked in debugger) ---
    action_set_priority(&action_5, 1);

    assert(sorted_set_item_priority(&action_1) == 1);
    assert(sorted_set_item_priority(&action_2) == 1);
    assert(sorted_set_item_priority(&action_3) == 1);
    assert(sorted_set_item_priority(&action_4) == 1);
    assert(sorted_set_item_priority(&action_5) == 1);

    // --- test local storage survived ---
    assert(process_current_local() == &process_1);

    process_1_executed = true;

    return 0;
}
