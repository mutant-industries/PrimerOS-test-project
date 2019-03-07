/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2018-2019 Mutant Industries ltd. */
#ifndef _TEST_DRIVER_IO_HANDLER_H_
#define _TEST_DRIVER_IO_HANDLER_H_

#include <test/common.h>
#include <test/driver/common.h>

/**
 * Assume there are jumpers connecting following pins:
 *  - P1.2 and P6.1
 *  - P1.3 and P5.2
 *  - P6.3 and P7.1
 */
void test_driver_io_handler(void);


#endif /* _TEST_DRIVER_IO_HANDLER_H_ */
