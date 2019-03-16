/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2018-2019 Mutant Industries ltd. */
#ifndef _TEST_KERNEL_STANDALONE_TIMING_H_
#define _TEST_KERNEL_STANDALONE_TIMING_H_

#include <test/common.h>
#include <test/driver/common.h>
#include <test/kernel/common.h>


/**
 * Timing subsystem functional test, might also be used as main entry point
 * - two periodic signals with nearly the same period
 * - process sleep
 * - blocking API timeout
 * - dynamic timing handle switch between default (16-bit TIMER_A) handle and alternative (10-bit TIMER_B) handle
 */
void test_kernel_standalone_timing(void);


#endif /* _TEST_KERNEL_STANDALONE_TIMING_H_ */
