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


#define assert(condition) if ( ! (condition)) test_fail()
#define assert_not(condition) if (condition) test_fail()

/**
 * Resource variable attribute - to allocate to FRAM place following to linker script:
 *  .resources  : {} > FRAM
 */
#define __resource \
    __attribute__((section(".resources")))


void test_fail(void);
void test_pass(void);
void expect_reset(void);
void power_on_reset(void);
void LPM_x_5_enter(void);

void WDT_disable(void);

/**
 * DCO_range_select: 0|DCORSEL
 * DCO_frequency_select: DCOFSEL_0|DCOFSEL_1|DCOFSEL_2|DCOFSEL_3|DCOFSEL_4|DCOFSEL_5|DCOFSEL_6|DCOFSEL_7
 * SMCLK_divider: DIVM__1|DIVM__2|DIVM__4|DIVM__8|DIVM__16|DIVM__32
 */
void default_clock_setup(uint16_t DCO_range_select, uint16_t DCO_frequency_select, uint16_t SMCLK_divider);

#endif /* _TEST_COMMON_H_ */
