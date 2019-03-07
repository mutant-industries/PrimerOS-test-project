/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2018-2019 Mutant Industries ltd. */
#ifndef _TEST_KERNEL_STANDALONE_WAKEUP_H_
#define _TEST_KERNEL_STANDALONE_WAKEUP_H_

#include <test/common.h>
#include <test/driver/common.h>
#include <test/kernel/common.h>

/**
 * Wakeup from LPM4.5 functional test, might also be used as main entry point
 *  - IO drivers reinit on wakeup
 *  - system persistent state reinit
 *  - timing subsystem reinit on wakeup
 *  - power save on idle
 */
void test_kernel_standalone_wakeup(void);


#endif /* _TEST_KERNEL_STANDALONE_WAKEUP_H_ */
