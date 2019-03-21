// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/crc/consumer.h>

// -------------------------------------------------------------------------------------

static __aligned(2) uint8_t test_data_1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };
static __aligned(2) uint8_t test_data_2[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

static uint8_t BSL_test_packet[] = { 0x17, 0x00, 0x44, 0x00 };
static uint16_t BSL_test_packet_crc = 0x0F42;

// -------------------------------------------------------------------------------------

void test_driver_crc_consumer() {
    uint16_t result;

    WDT_disable();
    default_clock_setup(0, DCOFSEL_0, DIVM__1);

    // CRC driver backed by hardware module
    CRC_driver_register(&CRC_driver_1, false);
    // CRC driver backed by software fallback
    CRC_driver_register(&CRC_driver_2, true);

    // --- CRC calculate test ---
    result = CRC_calculate(&CRC_driver_1, test_data_1, sizeof(test_data_1), 0xFFFF);

    assert(result == 0x29B1);

    result = CRC_calculate(&CRC_driver_2, test_data_1, sizeof(test_data_1), 0xFFFF);

    assert(result == 0x29B1);

    result = CRC_calculate(&CRC_driver_1, test_data_1, sizeof(test_data_1), 0x0000);

    assert(result == 0x31C3);

    result = CRC_calculate(&CRC_driver_2, test_data_1, sizeof(test_data_1), 0x0000);

    assert(result == 0x31C3);

    // --- CRC calculate test - not aligned memory block ---
    result = CRC_calculate(&CRC_driver_1, &test_data_2[1], sizeof(test_data_2) - 1, 0xFFFF);

    assert(result == 0x29B1);

    result = CRC_calculate(&CRC_driver_2, &test_data_2[1], sizeof(test_data_2) - 1, 0xFFFF);

    assert(result == 0x29B1);

    result = CRC_calculate(&CRC_driver_1, &test_data_2[1], sizeof(test_data_2) - 1, 0x0000);

    assert(result == 0x31C3);

    result = CRC_calculate(&CRC_driver_2, &test_data_2[1], sizeof(test_data_2) - 1, 0x0000);

    assert(result == 0x31C3);

    // --- CRC calculate test on BSL sample packet ---
    result = CRC_calculate(&CRC_driver_1, BSL_test_packet, sizeof(BSL_test_packet), 0xFFFF);

    assert(result == BSL_test_packet_crc);

    result = CRC_calculate(&CRC_driver_2, BSL_test_packet, sizeof(BSL_test_packet), 0xFFFF);

    assert(result == BSL_test_packet_crc);
}
