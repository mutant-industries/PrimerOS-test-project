// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/event/inheritance.h>
#include <action.h>

#define __TEST_SIGNAL__                         signal(0xA055)

// -------------------------------------------------------------------------------------

static bool test_action_1_handler(Action_t *, signal_t signal);
static bool test_action_2_handler(Action_t *, signal_t signal);
static bool test_action_3_handler(Action_t *, signal_t signal);
static bool test_action_4_handler(Action_t *, signal_t signal);

static volatile bool action_1_triggered, action_2_triggered, action_3_triggered, action_4_triggered;
static signal_t action_1_expected_signal;

// -------------------------------------------------------------------------------------

static void init() {

#ifndef __SIGNAL_PROCESSOR_DISABLE__
    // create test event to be executed within event processor
    event_create(&event_1);
    event_create(&event_2);

    action_create(&action_1, NULL, action_default_trigger, test_action_1_handler);
    action_create(&action_2, NULL, action_default_trigger, test_action_2_handler);
    action_create(&action_3, NULL, action_default_trigger, test_action_3_handler);
    action_create(&action_4, NULL, action_default_trigger, test_action_4_handler);

    action_owner(&action_1) = &action_1;
    action_owner(&action_2) = &action_2;
    action_owner(&action_3) = &action_3;
    action_owner(&action_4) = &action_4;

    sorted_set_item_priority(&action_1) = 1;
    sorted_set_item_priority(&action_2) = 2;
    sorted_set_item_priority(&action_3) = 3;
    sorted_set_item_priority(&action_4) = 4;

    event_subscribe(&event_1, &action_1);
    event_subscribe(&event_1, &action_2);

    event_subscribe(&event_2, &action_3);
    event_subscribe(&event_2, &action_4);

#endif
}

void test_kernel_event_inheritance() {

#ifndef __SIGNAL_PROCESSOR_DISABLE__
    WDT_disable();

    action_1_triggered = action_2_triggered = action_3_triggered = action_4_triggered = false;

    // --- create and schedule process from current context ---
    default_kernel_start(2, init, false);

    assert(sorted_set_item_priority(&event_1) == 2);
    assert(sorted_set_item_priority(&event_2) == 4);

    // --- test trigger event with the same priority as current process ---
    event_trigger(&event_1, __TEST_SIGNAL__);

    assert_not(action_1_triggered);
    assert_not(action_2_triggered);

    // --- test trigger event with higher priority than current process ---
    event_trigger(&event_2, __TEST_SIGNAL__);

    assert(action_3_triggered);
    assert(action_4_triggered);

    // --- test lower priority event still not handled ---
    assert_not(action_1_triggered);
    assert_not(action_2_triggered);

    yield();

    // --- test event_trigger() only triggers actions with higher or the same priority as highest priority process ---
    assert(action_2_triggered);
    assert_not(action_1_triggered);

    action_3_triggered = action_4_triggered = false;

    // --- test lower priority event handler finishes if started before higher priority event is triggered ---
    action_1_expected_signal = __TEST_SIGNAL__;

    event_subscribe(&event_2, &action_3);
    event_subscribe(&event_2, &action_4);

    event_trigger(&event_2, __TEST_SIGNAL__);

    assert(action_1_triggered);

    assert(action_3_triggered);
    assert(action_4_triggered);

    action_1_triggered = action_2_triggered = false;

    // --- test subscribed action priority is inherited by event processor ---
    event_subscribe(&event_1, &action_1);
    event_subscribe(&event_1, &action_2);

    event_trigger(&event_1, __TEST_SIGNAL__);

    assert_not(action_1_triggered);
    assert_not(action_2_triggered);

    yield();

    assert(action_2_triggered);
    assert_not(action_1_triggered);

    action_set_priority(&action_1, 2);

    assert_not(action_1_triggered);

    action_set_priority(&action_1, 3);

    assert(action_1_triggered);

    action_1_triggered = action_2_triggered = false;
    action_set_priority(&action_1, 1);

    // --- test action always triggered when event disposed ---
    action_1_expected_signal = EVENT_DISPOSED;

    event_subscribe(&event_1, &action_1);
    event_subscribe(&event_1, &action_2);

    event_trigger(&event_1, __TEST_SIGNAL__);

    yield();

    assert(action_2_triggered);
    assert_not(action_1_triggered);

    dispose(&event_1);

    assert(action_1_triggered);

#endif
}

static bool test_action_1_handler(Action_t *owner, signal_t signal) {
    assert(owner == &action_1);
    // --- test action_1 is never triggered because there is always some process with higher priority running  ---
    assert(signal == action_1_expected_signal);

    assert(sorted_set_item_priority(&event_1) == sorted_set_item_priority(owner));

    action_1_triggered = true;

    // remove action from subscription list
    return false;
}

static bool test_action_2_handler(Action_t *owner, signal_t signal) {
    assert(owner == &action_2);
    assert(signal == __TEST_SIGNAL__);

    assert(sorted_set_item_priority(&event_1) == sorted_set_item_priority(owner));

    action_2_triggered = true;

    // remove action from subscription list
    return false;
}

static bool test_action_3_handler(Action_t *owner, signal_t signal) {
    assert(owner == &action_3);
    assert(signal == __TEST_SIGNAL__);

    assert(sorted_set_item_priority(&event_2) == sorted_set_item_priority(owner));

    action_3_triggered = true;

    // remove action from subscription list
    return false;
}


static bool test_action_4_handler(Action_t *owner, signal_t signal) {
    assert(owner == &action_4);
    assert(signal == __TEST_SIGNAL__);

    assert(sorted_set_item_priority(&event_2) == sorted_set_item_priority(owner));

    action_4_triggered = true;

    // remove action from subscription list
    return false;
}
