// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/sync/barrier.h>
#include <kernel.h>
#include <process.h>
#include <scheduler.h>
#include <sync/semaphore.h>
#include <sync/mutex.h>


#define __INIT_PROCESS_PRIORITY__               0
#define __TEST_PROCESS_1_PRIORITY__             0
#define __TEST_PROCESS_2_PRIORITY__             0
#define __TEST_PROCESS_2_PRIORITY_SHIFT__       1

#define __TEST_SIGNAL__                         signal(0xA5A5)
#define __TEST_PROCESS_1_RETURN_SIGNAL__        signal(0xA0A0)
#define __TEST_PROCESS_2_RETURN_SIGNAL__        signal(0x0A0A)

#define _action_queue_attr arg_1

// -------------------------------------------------------------------------------------

static signal_t test_process_1_entry_point(Semaphore_t *, Mutex_t *);
static signal_t test_process_2_entry_point(Semaphore_t *, Mutex_t *);

static void barrier_trigger(Action_t *, signal_t signal);

static volatile bool process_1_started;
static volatile bool process_2_started;

static volatile bool process_1_mutex_acquired;
static volatile bool process_2_mutex_acquired;

// -------------------------------------------------------------------------------------

static void init() {

#ifndef __SIGNAL_PROCESSOR_DISABLE__
    // initialize action queue where two processes shall be placed after initialized
    action_queue_create(&action_queue_1, true);
    // initialize action proxy, that shall schedule both processes
    action_create(&action_1, NULL, barrier_trigger);

    action_attr(&action_1, _action_queue_attr) = &action_queue_1;
    // initialize test semaphore
    semaphore_create(&semaphore_1, 0);
    // initialize test mutex
    mutex_register(&mutex_1);

    // proxy trigger on semaphore signal
    assert( ! semaphore_acquire_async(&semaphore_1, &action_1));

    // initialize both processes
    zerofill(&process_1.create_config);
    zerofill(&process_2.create_config);

    process_1.create_config.priority = __TEST_PROCESS_1_PRIORITY__;
    process_1.create_config.stack_addr_low = (data_pointer_register_t) test_stack_1;
    process_1.create_config.stack_size = __TEST_STACK_SIZE__;
    process_1.create_config.entry_point = (process_entry_point_t) test_process_1_entry_point;
    process_1.create_config.arg_1 = &semaphore_1;
    process_1.create_config.arg_2 = &mutex_1;

    // --- create process with lower priority than init process ---
    process_create(&process_1);

    process_2.create_config.priority = __TEST_PROCESS_2_PRIORITY__;
    process_2.create_config.stack_addr_low = (data_pointer_register_t) test_stack_2;
    process_2.create_config.stack_size = __TEST_STACK_SIZE__;
    process_2.create_config.entry_point = (process_entry_point_t) test_process_2_entry_point;
    process_2.create_config.arg_1 = &semaphore_1;
    process_2.create_config.arg_2 = &mutex_1;

    // --- create process with lower priority than init process ---
    process_create(&process_2);

    // place both processes to test queue
    action_queue_insert(&action_queue_1, &process_1);
    action_queue_insert(&action_queue_1, &process_2);

#endif
}

void test_kernel_sync_barrier() {

#ifndef __SIGNAL_PROCESSOR_DISABLE__
    WDT_disable();

    process_1_started = process_2_started = false;
    process_1_mutex_acquired = process_2_mutex_acquired = false;

    // --- create and schedule process from current context ---
    default_kernel_start(__INIT_PROCESS_PRIORITY__, init, false);

    assert_not(process_1_started || process_2_started);

    // --- test semaphore signal, both processes should be scheduled ---
    semaphore_signal(&semaphore_1, __TEST_SIGNAL__);

    assert_not(process_1_started || process_2_started);

    // --- start test ---
    assert(process_wait(&process_1) == __TEST_PROCESS_1_RETURN_SIGNAL__);

    assert(process_1_started && process_2_started);

    // --- test semaphore signal passing ---
    semaphore_signal(&semaphore_1, __TEST_SIGNAL__);

    // --- test priority shift, process_2 should be terminated by now ---
    assert(process_wait(&process_2) == PROCESS_SIGNAL_EXIT);

    // --- test mutex has no owner ---
    assert( ! mutex_try_lock(&mutex_1));

    // kernel halt
    dispose(&init_process);
    // driver release
    dispose(&timer_driver_1);

#endif
}

static signal_t test_process_1_entry_point(Semaphore_t *semaphore, Mutex_t *mutex) {
    process_1_started = true;

    assert(semaphore == &semaphore_1);
    assert(mutex == &mutex_1);

    // --- test process_1 started before process_2 ---
    assert_not(process_2_started);

    assert( ! mutex_lock(mutex));

    process_1_mutex_acquired = true;

    yield();    // -> context switch

    // --- test yield, test process_2 blocks on mutex ---
    assert(process_2_started && ! process_2_mutex_acquired);

    // --- test mutex reentrant behavior ---
    assert( ! mutex_try_lock(mutex));
    assert( ! mutex_unlock(mutex));

    assert_not(process_2_mutex_acquired);

    // --- test mutex release ---
    assert( ! mutex_unlock(mutex));

    // --- test context not switched after mutex release ---
    assert_not(process_2_mutex_acquired);

    yield();    // -> context switch

    assert(process_2_mutex_acquired);

    return __TEST_PROCESS_1_RETURN_SIGNAL__;
}

static signal_t test_process_2_entry_point(Semaphore_t *semaphore, Mutex_t *mutex) {
    process_2_started = true;

    assert(semaphore == &semaphore_1);
    assert(mutex == &mutex_1);

    // --- test process_1 started before process_2 ---
    assert(process_1_started && process_1_mutex_acquired);

    // --- test process_1 already acquired mutex ---
    assert(mutex_try_lock(mutex));

    // --- test blocking state on mutex ---
    assert( ! mutex_lock(mutex));    // -> context switch

    process_2_mutex_acquired = true;

    // --- test blocking state on semaphore with priority shift ---
    assert(semaphore_acquire(semaphore, NULL, &(Schedule_config_t){__TEST_PROCESS_2_PRIORITY_SHIFT__}) == __TEST_SIGNAL__); // -> context switch

    assert(sorted_set_item_priority(running_process) == __TEST_PROCESS_2_PRIORITY_SHIFT__);

    // --- test terminate without releasing lock ---
    return __TEST_PROCESS_2_RETURN_SIGNAL__;
}

// -------------------------------------------------------------------------------------

static void barrier_trigger(Action_t *_this, signal_t signal) {
    // simple action proxy that triggers queue
    action_queue_trigger_all(action_queue(action_attr(_this, _action_queue_attr)), signal);
}
