// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/sync/inversion.h>


// -------------------------------------------------------------------------------------

static signal_t test_process_1_entry_point(Mutex_t *, Schedule_config_t *);
static signal_t test_process_2_entry_point(Mutex_t *, Schedule_config_t *);

static volatile bool process_1_started;
static volatile bool process_2_started;

static volatile bool process_1_mutex_acquired;
static volatile bool process_2_mutex_acquired;

// -------------------------------------------------------------------------------------

static void init() {
    // initialize test mutex
    mutex_register(&mutex_1);

    // initialize two processes
    zerofill(&process_1.create_config);
    zerofill(&process_2.create_config);

    process_1.create_config.stack_addr_low = (data_pointer_register_t) test_stack_1;
    process_1.create_config.stack_size = __TEST_STACK_SIZE__;
    process_1.create_config.entry_point = (process_entry_point_t) test_process_1_entry_point;
    process_1.create_config.arg_1 = &mutex_1;

    process_2.create_config.stack_addr_low = (data_pointer_register_t) test_stack_2;
    process_2.create_config.stack_size = __TEST_STACK_SIZE__;
    process_2.create_config.entry_point = (process_entry_point_t) test_process_2_entry_point;
    process_2.create_config.arg_1 = &mutex_1;
}

void test_kernel_sync_inversion() {

    WDT_disable();

    process_1_started = process_2_started = false;
    process_1_mutex_acquired = process_2_mutex_acquired = false;

    zerofill(&init_process_schedule_config);
    zerofill(&process_1_schedule_config);

    // --- create and schedule process from current context ---
    default_kernel_start(0, init, false);

    process_1.create_config.priority = 1;
    process_2.create_config.priority = 2;

    // --- create process with higher priority than init process ---
    process_create(&process_1);
    // --- create process with higher priority than init process ---
    process_create(&process_2);

    assert(process_is_alive(&process_1) && process_is_alive(&process_2));

    assert( ! mutex_try_lock(&mutex_1));

    // --- test priority unchanged after mutex acquired ---
    assert(sorted_set_item_priority(running_process) == 0);

    assert_not(process_1_started || process_2_started);

    // --- test priority inherited from process_2 ---
    process_schedule(&process_2, 0);

    assert(process_2_started && ! process_2_mutex_acquired);

    assert(sorted_set_item_priority(running_process) == 2);

    // --- test kill process, that is in a mutex queue ---
    process_kill(&process_2);

    assert(sorted_set_item_priority(running_process) == 0);

    // ------------------------

    process_1_started = process_2_started = false;

    process_create(&process_2);

    // --- test priority inherited from process_1 when process_2 is killed ---
    process_schedule(&process_1, 0);

    assert(process_1_started && ! process_1_mutex_acquired);

    assert(sorted_set_item_priority(running_process) == 1);

    process_schedule(&process_2, 0);

    assert(process_2_started && ! process_2_mutex_acquired);

    assert(sorted_set_item_priority(running_process) == 2);

    process_kill(&process_2);

    assert(sorted_set_item_priority(running_process) == 1);

    // --- test priority reset when mutex released ---
    assert( ! mutex_unlock(&mutex_1));

    assert(process_1_mutex_acquired);
    assert(sorted_set_item_priority(running_process) == 0);

    // ------------------------

    process_1_started = process_2_started = false;
    process_1_mutex_acquired = process_2_mutex_acquired = false;

    process_1.create_config.arg_2 = signal(&process_1_schedule_config);

    process_1.create_config.priority = 3;
    process_2.create_config.priority = 3;

    process_create(&process_1);
    process_create(&process_2);

    init_process_schedule_config.priority = 2;
    process_1_schedule_config.priority = 4;

    // --- test lock with schedule config ---
    assert( ! mutex_lock(&mutex_1, &init_process_schedule_config));

    assert(sorted_set_item_priority(running_process) == 2);

    process_schedule(&process_2, 0);

    assert(process_2_started && ! process_2_mutex_acquired);

    assert(sorted_set_item_priority(running_process) == 3);

    process_kill(&process_2);

    assert(sorted_set_item_priority(running_process) == 2);

    process_schedule(&process_1, 0);

    assert(process_1_started && ! process_1_mutex_acquired);

    assert(sorted_set_item_priority(running_process) == 4);

    process_kill(&process_1);

    assert(sorted_set_item_priority(running_process) == 2);

    assert( ! mutex_unlock(&mutex_1));

    assert(sorted_set_item_priority(running_process) == 2);

    yield();

    assert(sorted_set_item_priority(running_process) == 0);

    // ------------------------

    process_1_started = process_2_started = false;
    process_1_mutex_acquired = process_2_mutex_acquired = false;

    process_create(&process_1);
    process_create(&process_2);

    // --- test release lock and keep requested priority ---
    assert( ! mutex_lock(&mutex_1, &init_process_schedule_config));

    assert(sorted_set_item_priority(running_process) == 2);

    process_schedule(&process_1, 0);

    assert(process_1_started && ! process_1_mutex_acquired);

    assert(sorted_set_item_priority(running_process) == 4);

    process_schedule(&process_2, 0);

    assert_not(process_2_started);

    assert(sorted_set_item_priority(running_process) == 4);

    process_kill(&process_1);

    assert(process_2_started && ! process_2_mutex_acquired);
    assert(sorted_set_item_priority(running_process) == 3);

    assert( ! mutex_unlock(&mutex_1));

    assert(process_2_mutex_acquired);

    assert(sorted_set_item_priority(running_process) == 2);

    yield();

    assert(sorted_set_item_priority(running_process) == 0);

    // kernel halt
    dispose(&init_process);
    // driver release
    dispose(&timer_driver);
}

static signal_t test_process_1_entry_point(Mutex_t *mutex, Schedule_config_t *lock_with) {
    process_1_started = true;

    assert(mutex_unlock(mutex) == MUTEX_INVALID_OWNER);

    assert(mutex == &mutex_1);

    assert( ! mutex_lock(mutex, lock_with));

    process_1_mutex_acquired = true;

    return 0;
}

static signal_t test_process_2_entry_point(Mutex_t *mutex, Schedule_config_t *lock_with) {
    process_2_started = true;

    assert(mutex_unlock(mutex) == MUTEX_INVALID_OWNER);

    assert(mutex == &mutex_1);

    assert( ! mutex_lock(mutex, lock_with));

    process_2_mutex_acquired = true;

    return 0;
}
