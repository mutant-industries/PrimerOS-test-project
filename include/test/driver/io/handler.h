/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2018-2019 Mutant Industries ltd. */
#ifndef _TEST_DRIVER_IO_HANDLER_H_
#define _TEST_DRIVER_IO_HANDLER_H_

#include <test/common.h>
#include <test/driver/common.h>

/**
 * Assume there are jumpers connecting following pins:
 *  - P1.4 and P3.3
 *  - P1.5 and P4.7
 *  - P3.6 and P5.7
 */
void test_driver_io_handler(void);


#endif /* _TEST_DRIVER_IO_HANDLER_H_ */
