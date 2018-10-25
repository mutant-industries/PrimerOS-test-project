/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  MSP430 test framework common support functions
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _TEST_COMMON_H_
#define _TEST_COMMON_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <test/debug.h>


void test_fail(void);
void test_pass(void);
void expect_reset(void);
void power_on_reset(void);

void WDT_hold(void);
void default_clock_setup(void);

#endif /* _TEST_COMMON_H_ */
