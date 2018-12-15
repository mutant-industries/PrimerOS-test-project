/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  MSP430 minimalistic test framework for FRAM devices
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _TEST_RUNNER_H_
#define _TEST_RUNNER_H_

#include <test/common.h>
#include <test/driver/wdt/disabled.h>
#include <test/driver/wdt/watchdog.h>
#include <test/driver/wdt/interval.h>
#include <test/driver/vector/trigger.h>
#include <test/driver/timer/dispose.h>
#include <test/driver/timer/multiple.h>
#include <test/driver/stack/pointer.h>
#include <test/driver/stack/deferred.h>
#include <test/kernel/dispose/chain.h>
#include <test/kernel/dispose/resource.h>
#include <test/kernel/process/lifecycle.h>
#include <test/kernel/queue/sorting.h>
#include <test/kernel/queue/fifo.h>
#include <test/kernel/queue/priority.h>
#include <test/kernel/sync/barrier.h>
#include <test/kernel/sync/inversion.h>
#include <test/kernel/event/trigger.h>
#include <test/kernel/event/inheritance.h>


/**
 * Test framework entry point, might also be user as main entry point
 *  1. run single test
 *  2. restart
 *  3. check that previous test did not crash (executed test count == passed test count)
 *  4. run next test
 *  5. ... if no more tests are to be ran, signal end by green LED and restart the whole process
 */
void test_runner_all(void);
void test_runner_single(void(*)(void));


#endif /* _TEST_RUNNER_H_ */
