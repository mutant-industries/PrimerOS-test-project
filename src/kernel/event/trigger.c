// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/event/trigger.h>
#include <action/signal.h>

#define __TEST_SIGNAL__                         signal(0x50A5)

// -------------------------------------------------------------------------------------

static signal_t test_process_1_entry_point(void);
static signal_t test_process_2_entry_point(void);
static bool test_subscription_1_on_publish_handler(Subscription_t *, signal_t signal);
static bool test_subscription_2_on_publish_handler(Subscription_t *, signal_t signal);
static bool test_subscription_1_signal_interceptor(Subscription_t *owner, signal_t *signal);
static bool test_subscription_2_signal_interceptor(Subscription_t *owner, signal_t *signal);

static volatile bool process_1_started;
static volatile bool process_2_started;

static volatile bool process_1_wait_loop_ended;
static volatile bool process_1_event_blocking_wait_ended;
static volatile bool process_2_wait_loop_ended;
static volatile bool process_2_event_blocking_wait_ended;

static volatile uint16_t test_subscription_1_signal_interceptor_call_count;
static volatile uint16_t test_subscription_1_on_publish_handler_call_count;
static volatile uint16_t test_subscription_2_signal_interceptor_call_count;
static volatile uint16_t test_subscription_2_on_publish_handler_call_count;

static volatile bool handler_next_return_value, interceptor_next_return_value;

// -------------------------------------------------------------------------------------

static void init() {

#ifndef __SIGNAL_PROCESSOR_DISABLE__
    // create test event to be executed within event processor
    event_create(&event_1);
#endif

    // initialize both processes
    zerofill(&process_1.create_config);
    zerofill(&process_2.create_config);

    process_1.create_config.priority = 0;
    process_1.create_config.stack_addr_low = (data_pointer_register_t) test_stack_1;
    process_1.create_config.stack_size = __TEST_STACK_SIZE__;
    process_1.create_config.entry_point = (process_entry_point_t) test_process_1_entry_point;

    // --- create process with the same priority as init process ---
    process_create(&process_1);

    process_2.create_config.priority = 0;
    process_2.create_config.stack_addr_low = (data_pointer_register_t) test_stack_2;
    process_2.create_config.stack_size = __TEST_STACK_SIZE__;
    process_2.create_config.entry_point = (process_entry_point_t) test_process_2_entry_point;

    // --- create process with the same priority as init process ---
    process_create(&process_2);
}

void test_kernel_event_trigger() {

#ifndef __SIGNAL_PROCESSOR_DISABLE__
    WDT_disable();

    process_1_started = process_2_started = false;
    process_1_wait_loop_ended = process_2_wait_loop_ended = false;
    process_1_event_blocking_wait_ended = process_2_event_blocking_wait_ended = false;
    test_subscription_1_signal_interceptor_call_count = test_subscription_2_signal_interceptor_call_count = 0;
    test_subscription_1_on_publish_handler_call_count = test_subscription_2_on_publish_handler_call_count = 0;
    handler_next_return_value = interceptor_next_return_value = true;

    // --- create and schedule process from current context ---
    default_kernel_start(&init_process, 0, init, false);

    process_schedule(&process_1, 0);
    process_schedule(&process_2, 0);

    assert_not(process_1_started || process_2_started);

    yield();

    assert(process_1_started && process_2_started);
    assert( ! test_subscription_1_on_publish_handler_call_count);
    assert( ! test_subscription_2_on_publish_handler_call_count);

    // --- test signal pass ---
    event_trigger(&event_1, __TEST_SIGNAL__);

    // --- test context did not switch to process_1 yet ---
    assert(test_subscription_1_on_publish_handler_call_count == 0);
    assert(test_subscription_1_signal_interceptor_call_count == 1);

    // --- test process_2 dynamic priority on subscription trigger ---
    assert(test_subscription_2_on_publish_handler_call_count == 1);
    assert(test_subscription_2_signal_interceptor_call_count == 1);

    yield();

    // --- test signal processed in process_1 ---
    assert(test_subscription_1_on_publish_handler_call_count == 1);
    assert(test_subscription_2_on_publish_handler_call_count == 1);

    // --- test signal filter ---
    interceptor_next_return_value = false;

    event_trigger(&event_1, __TEST_SIGNAL__);

    assert(test_subscription_1_on_publish_handler_call_count == 1);
    assert(test_subscription_1_signal_interceptor_call_count == 2);

    assert(test_subscription_2_on_publish_handler_call_count == 1);
    assert(test_subscription_2_signal_interceptor_call_count == 2);

    yield();

    assert(test_subscription_1_on_publish_handler_call_count == 1);
    assert(test_subscription_1_signal_interceptor_call_count == 2);

    assert(test_subscription_2_on_publish_handler_call_count == 1);
    assert(test_subscription_2_signal_interceptor_call_count == 2);

    interceptor_next_return_value = true;

    // --- test exit process waiting state ---
    handler_next_return_value = false;

    event_trigger(&event_1, __TEST_SIGNAL__);

    assert_not(process_1_wait_loop_ended);
    // process_2 priority was reset after signal was processed, therefore wait loop not ended yet
    assert_not(process_2_wait_loop_ended);

    assert(test_subscription_1_on_publish_handler_call_count == 1);
    assert(test_subscription_1_signal_interceptor_call_count == 3);

    assert(test_subscription_2_on_publish_handler_call_count == 2);
    assert(test_subscription_2_signal_interceptor_call_count == 3);

    yield();

    assert(process_1_wait_loop_ended && process_2_wait_loop_ended);

    assert(test_subscription_1_on_publish_handler_call_count == 2);
    assert(test_subscription_1_signal_interceptor_call_count == 3);

    assert(test_subscription_2_on_publish_handler_call_count == 2);
    assert(test_subscription_2_signal_interceptor_call_count == 3);

    assert_not(process_1_event_blocking_wait_ended || process_2_event_blocking_wait_ended);

    // --- test process blocking wait ---
    event_trigger(&event_1, __TEST_SIGNAL__);

    assert_not(process_1_event_blocking_wait_ended);
    assert(process_2_event_blocking_wait_ended);

    assert(test_subscription_1_on_publish_handler_call_count == 2);
    assert(test_subscription_1_signal_interceptor_call_count == 3);

    assert(test_subscription_2_on_publish_handler_call_count == 2);
    assert(test_subscription_2_signal_interceptor_call_count == 3);

    // yield just to allow event processor to finish scheduling process_1
    yield();

    assert_not(process_1_event_blocking_wait_ended);

    // yield to switch to process_1
    yield();

    assert(process_1_event_blocking_wait_ended);

#ifdef __RESOURCE_MANAGEMENT_ENABLE__
    // kernel halt - only if resource management is enabled, otherwise event processor is not terminated
    dispose(&init_process);
#endif
    // driver release
    dispose(&timer_driver_1);
#endif
}

static signal_t test_process_1_entry_point() {
    process_1_started = true;

    subscription_create(&subscription_1, test_subscription_1_on_publish_handler,
            &subscription_1, true, test_subscription_1_signal_interceptor);

    event_subscribe(&event_1, &subscription_1);

    signal_wait();

    process_1_wait_loop_ended = true;

    // --- test event dummy trigger setter ---
    dispose(&subscription_1);

    // --- test event blocking wait ---
    assert(event_wait(&event_1) == __TEST_SIGNAL__);

    process_1_event_blocking_wait_ended = true;

    return 0;
}

static signal_t test_process_2_entry_point() {
    process_2_started = true;

    schedule_config_reset(&process_1_schedule_config);

    process_1_schedule_config.priority = 1;

    subscription_create(&subscription_2, test_subscription_2_on_publish_handler,
            &subscription_2, true, test_subscription_2_signal_interceptor, &process_1_schedule_config);

    event_subscribe(&event_1, &subscription_2);

    signal_wait();

    process_2_wait_loop_ended = true;

    // --- test event dummy trigger setter ---
    dispose(&subscription_2);

    // --- test event blocking wait ---
    assert(event_wait(&event_1, NULL, &process_1_schedule_config) == __TEST_SIGNAL__);

    process_2_event_blocking_wait_ended = true;

    return 0;
}

// -------------------------------------------------------------------------------------

static bool test_subscription_1_on_publish_handler(Subscription_t *subscription, signal_t signal) {
    assert(subscription == &subscription_1);
    assert(signal == __TEST_SIGNAL__);

    test_subscription_1_on_publish_handler_call_count++;

    return handler_next_return_value;
}

static bool test_subscription_2_on_publish_handler(Subscription_t *subscription, signal_t signal) {
    assert(subscription == &subscription_2);
    assert(signal == __TEST_SIGNAL__);

    test_subscription_2_on_publish_handler_call_count++;

    return handler_next_return_value;
}

static bool test_subscription_1_signal_interceptor(Subscription_t *owner, signal_t *signal) {
    assert(owner == &subscription_1);
    assert(*signal == __TEST_SIGNAL__);

    test_subscription_1_signal_interceptor_call_count++;

    return interceptor_next_return_value;
}

static bool test_subscription_2_signal_interceptor(Subscription_t *owner, signal_t *signal) {
    assert(owner == &subscription_2);
    assert(*signal == __TEST_SIGNAL__);

    test_subscription_2_signal_interceptor_call_count++;

    return interceptor_next_return_value;
}
