/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2018-2019 Mutant Industries ltd. */
#ifndef _TEST_KERNEL_QUEUE_SORTING_H_
#define _TEST_KERNEL_QUEUE_SORTING_H_

#include <stdint.h>
#include <test/common.h>
#include <test/kernel/common.h>


void test_kernel_queue_sorting(void);

/**
 * Test consistent state of given sorted queue
 */
void test_sorted_queue(Action_queue_t *queue, uint16_t expected_size, Action_t *expected_in_queue, uint16_t expected_at_index,
        Action_t *expected_not_in_queue);


#endif /* _TEST_KERNEL_QUEUE_SORTING_H_ */
