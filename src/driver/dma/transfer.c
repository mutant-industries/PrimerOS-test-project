// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/dma/transfer.h>
#include <driver/interrupt.h>

// -------------------------------------------------------------------------------------

#define __TEST_MEMORY_BLOCK_SIZE__      0x3E

static void test_handler(DMA_channel_handle_t *);
static void DMA_channel_transfer_test(DMA_channel_handle_t *);

static uint8_t interrupt_handler_call_count;
static DMA_channel_handle_t *expected_channel_interrupt;

static uint8_t src_memory_block[__TEST_MEMORY_BLOCK_SIZE__];
static uint8_t dst_memory_block[__TEST_MEMORY_BLOCK_SIZE__];

#ifdef _DATA_MODEL_LARGE_
uintptr_t test_src_address_ptr = (uintptr_t) 0x1A5A5;
uintptr_t test_dst_address_ptr = (uintptr_t) 0x15A5A;
#endif

// -------------------------------------------------------------------------------------

void test_driver_dma_transfer() {

    WDT_disable();
    default_clock_setup(0, DCOFSEL_0, DIVM__1);

    // DMA driver init
    DMA_driver_register(&DMA_driver);

    interrupt_enable();

    // DMA test channels init
    assert(DMA_driver_channel_register(&DMA_driver, &DMA_handle_1, 0) == DMA_OK);
    assert(DMA_driver_channel_register(&DMA_driver, &DMA_handle_2, 1) == DMA_OK);
    assert(DMA_driver_channel_register(&DMA_driver, &DMA_handle_3, 2) == DMA_OK);
    assert(DMA_driver_channel_register(&DMA_driver, &DMA_handle_4, 3) == DMA_OK);
    assert(DMA_driver_channel_register(&DMA_driver, &DMA_handle_5, 4) == DMA_OK);

    // --- test DMA transfer initiated by SW request for each channel ---
    DMA_channel_transfer_test(&DMA_handle_1);
    DMA_channel_transfer_test(&DMA_handle_2);
    DMA_channel_transfer_test(&DMA_handle_3);
    DMA_channel_transfer_test(&DMA_handle_4);
    DMA_channel_transfer_test(&DMA_handle_5);

    // --- test register already registered channel ---
    assert(DMA_driver_channel_register(&DMA_driver, &DMA_handle_6, 4) == DMA_CHANNEL_REGISTERED_ALREADY);

    // --- test channel dispose ---
    interrupt_handler_call_count = 0;

    assert(DMA_channel_set_enabled(&DMA_handle_5, true) == DMA_OK);
    DMA_channel_request(&DMA_handle_5);

    // make sure IFG is not set next instruction
    __delay_cycles(6);

    assert(interrupt_handler_call_count == 1);

    dispose(&DMA_handle_5);

    assert(DMA_channel_set_enabled(&DMA_handle_5, true) == DMA_UNSUPPORTED_OPERATION);
    DMA_channel_request(&DMA_handle_5);

    // make sure IFG is not set next instruction
    __delay_cycles(6);

    assert(interrupt_handler_call_count == 1);

    // --- test API unavailable after disposed ---
    assert(DMA_channel_select_trigger(&DMA_handle_5, DMA5TSEL__DMAREQ) == DMA_UNSUPPORTED_OPERATION);
    assert(DMA_channel_set_control(&DMA_handle_5, 0, 0, 0, 0, 0, 0) == DMA_UNSUPPORTED_OPERATION);

    // --- test register channel that was just disposed ---
    assert(DMA_driver_channel_register(&DMA_driver, &DMA_handle_6, 4) == DMA_OK);

    DMA_channel_transfer_test(&DMA_handle_6);

#ifdef _DATA_MODEL_LARGE_
    // --- test 20-bit address write
    DMA_channel_source_address(&DMA_handle_6) = (void *) test_src_address_ptr;
    DMA_channel_destination_address(&DMA_handle_6) = (void *) test_dst_address_ptr;

    assert(DMA_channel_source_address(&DMA_handle_6) == (void *) test_src_address_ptr
           && DMA_channel_destination_address(&DMA_handle_6) == (void *) test_dst_address_ptr);
#endif

    // --- test driver control register access ---
    DMA_driver_set_control(&DMA_driver, ENNMI_0, ROUNDROBIN_1, DMARMWDIS_0);

    assert_not(DMACTL4 & ENNMI);
    assert(DMACTL4 & ROUNDROBIN);
    assert_not(DMACTL4 & DMARMWDIS);

    DMA_driver_set_control(&DMA_driver, ENNMI_1, ROUNDROBIN_0, DMARMWDIS_1);

    assert(DMACTL4 & ENNMI);
    assert_not(DMACTL4 & ROUNDROBIN);
    assert(DMACTL4 & DMARMWDIS);

    // --- test driver dispose ---
    interrupt_handler_call_count = 0;

    dispose(&DMA_driver);

    assert(DMA_channel_set_enabled(&DMA_handle_1, true) == DMA_UNSUPPORTED_OPERATION);
    DMA_channel_request(&DMA_handle_1);

    // make sure IFG is not set next instruction
    __delay_cycles(6);

    assert(interrupt_handler_call_count == 0);

    // --- test driver control reset on dispose ---
    assert_not(DMACTL4 & ENNMI);
    assert_not(DMACTL4 & ROUNDROBIN);
    assert_not(DMACTL4 & DMARMWDIS);
}

// -------------------------------------------------------------------------------------

static void DMA_channel_transfer_test(DMA_channel_handle_t *handle) {
    uint16_t i;

    interrupt_handler_call_count = 0;
    expected_channel_interrupt = handle;

    // DMA channel interrupt handler register, set enabled
    assert(vector_register_handler(handle, test_handler, handle, NULL) != NULL);
    assert(vector_set_enabled(handle, true) == VECTOR_OK);

    // DMA control set edge-sensitive block transfer
    assert(DMA_channel_set_control(handle, DMALEVEL__EDGE, DMASRCBYTE__BYTE, DMADSTBYTE__BYTE, DMASRCINCR_3, DMADSTINCR_3, DMADT_1) == DMA_OK);

    // DMA channel trigger on software request
    assert(DMA_channel_select_trigger(handle, DMA5TSEL__DMAREQ) == DMA_OK);

    // DMA address and size setup
    DMA_channel_source_address(handle) = &src_memory_block;
    DMA_channel_destination_address(handle) = &dst_memory_block;
    DMA_channel_size(handle) = __TEST_MEMORY_BLOCK_SIZE__;

    // enable DMA transfer on trigger
    assert(DMA_channel_set_enabled(handle, true) == DMA_OK);

    // prepare test memory blocks
    for (i = 0; i < __TEST_MEMORY_BLOCK_SIZE__; i++) {
        src_memory_block[i] = (uint8_t) i;
        dst_memory_block[i] = 0;
    }

    assert(interrupt_handler_call_count == 0);

    // request DMA transfer
    DMA_channel_request(handle);

    // make sure IFG is not set next instruction
    __delay_cycles(6);

    assert(interrupt_handler_call_count == 1);

    // --- test memory was transferred correctly ---
    for (i = 0; i < __TEST_MEMORY_BLOCK_SIZE__; i++) {
        assert(src_memory_block[i] == dst_memory_block[i]);
    }

    // assert DMA channel is no longer enabled

    // prepare test memory blocks
    for (i = 0; i < __TEST_MEMORY_BLOCK_SIZE__; i++) {
        src_memory_block[i] = (uint8_t) i;
        dst_memory_block[i] = 0;
    }

    // request DMA transfer
    DMA_channel_request(handle);

    // make sure IFG is not set next instruction
    __delay_cycles(6);

    assert(interrupt_handler_call_count == 1);

    // --- test no memory transfer was triggered when handle is disabled ---
    for (i = 0; i < __TEST_MEMORY_BLOCK_SIZE__; i++) {
        assert(dst_memory_block[i] == 0);
    }

    // cancel request that shall not be processed
    DMA_channel_request_cancel(handle);
}

static void test_handler(DMA_channel_handle_t *handle) {
    assert(handle == expected_channel_interrupt);

    interrupt_handler_call_count++;
}
