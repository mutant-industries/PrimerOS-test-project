// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/dma/uart.h>
#include <driver/interrupt.h>

// -------------------------------------------------------------------------------------

#define __TEST_UART_NO__   0
#define __TEST_MEMORY_BLOCK_SIZE__      0x3E

static void RX_channel_handler(DMA_channel_handle_t *);
static void TX_channel_handler(DMA_channel_handle_t *);

static volatile uint8_t TX_channel_handler_call_count;
static volatile uint8_t RX_channel_handler_call_count;

static uint8_t src_memory_block[__TEST_MEMORY_BLOCK_SIZE__];
static uint8_t dst_memory_block[__TEST_MEMORY_BLOCK_SIZE__];

// -------------------------------------------------------------------------------------

void test_driver_dma_uart() {
    UART_baudrate_config_t baudrate_config;
    uint16_t i;

    WDT_disable();
    // DCO 8MHz, SMCLK input DCO / 1
    default_clock_setup(0, DCOFSEL_6, DIVM__1);

    TX_channel_handler_call_count = RX_channel_handler_call_count = 0;

    interrupt_enable();

    // --- UART init ---

    // UART driver init
    UART_driver_register(&UART_driver_1, UART_BASE(__TEST_UART_NO__), UART_VECTOR(__TEST_UART_NO__));

    // SMCLK (8MHz), baudrate 9600
    baudrate_config.clock_source = UCSSEL__SMCLK;
    baudrate_config.clock_prescaler = 52;
    baudrate_config.first_modulation_stage = 1;
    baudrate_config.second_modulation_stage = 0x49;
    baudrate_config.oversampling = true;

    UART_set_baudrate_config(&UART_driver_1, &baudrate_config);
    UART_set_loopback(&UART_driver_1, true);

    // --- DMA init ---

    // DMA driver init
    DMA_driver_register(&DMA_driver);

    // TX channel
    DMA_driver_channel_register(&DMA_driver, &DMA_handle_1, 0);
    DMA_channel_set_control(&DMA_handle_1, DMALEVEL__EDGE, DMASRCBYTE__BYTE, DMADSTBYTE__BYTE, DMASRCINCR_3, DMADSTINCR_0, DMADT_0);

    DMA_channel_select_trigger(&DMA_handle_1, DMA0TSEL__UCA0TXIFG);

    DMA_channel_source_address(&DMA_handle_1) = &src_memory_block;
    DMA_channel_destination_address(&DMA_handle_1) = UART_TX_buffer_address(&UART_driver_1);
    DMA_channel_size(&DMA_handle_1) = __TEST_MEMORY_BLOCK_SIZE__;

    vector_register_handler(&DMA_handle_1, TX_channel_handler, &DMA_handle_1, NULL);
    vector_set_enabled(&DMA_handle_1, true);

    DMA_channel_set_enabled(&DMA_handle_1, true);

    // RX channel
    DMA_driver_channel_register(&DMA_driver, &DMA_handle_2, 1);
    DMA_channel_set_control(&DMA_handle_2, DMALEVEL__EDGE, DMASRCBYTE__BYTE, DMADSTBYTE__BYTE, DMASRCINCR_0, DMADSTINCR_3, DMADT_0);

    DMA_channel_select_trigger(&DMA_handle_2, DMA1TSEL__UCA0RXIFG);

    DMA_channel_source_address(&DMA_handle_2) = UART_RX_buffer_address(&UART_driver_1);
    DMA_channel_destination_address(&DMA_handle_2) = &dst_memory_block;
    DMA_channel_size(&DMA_handle_2) = __TEST_MEMORY_BLOCK_SIZE__;

    vector_register_handler(&DMA_handle_2, RX_channel_handler, &DMA_handle_2, NULL);
    vector_set_enabled(&DMA_handle_2, true);

    DMA_channel_set_enabled(&DMA_handle_2, true);

    // prepare test memory blocks
    for (i = 0; i < __TEST_MEMORY_BLOCK_SIZE__; i++) {
        src_memory_block[i] = (uint8_t) i;
        dst_memory_block[i] = 0;
    }

    assert(TX_channel_handler_call_count == 0);
    assert(RX_channel_handler_call_count == 0);

    // --- start test ---

    // enable UART, transmit buffer empty IFG should be set, which should trigger DMA transfer
    UART_reset_disable(&UART_driver_1);

    // wait for transfer end
    while (TX_channel_handler_call_count == 0 || RX_channel_handler_call_count == 0);

    // --- test memory was transferred correctly ---
    for (i = 0; i < __TEST_MEMORY_BLOCK_SIZE__; i++) {
        assert(src_memory_block[i] == dst_memory_block[i]);
    }

    dispose(&DMA_driver);
    dispose(&UART_driver_1);
}

static void TX_channel_handler(DMA_channel_handle_t *handle) {
    assert(handle == &DMA_handle_1);

    TX_channel_handler_call_count++;
}

static void RX_channel_handler(DMA_channel_handle_t *handle) {
    assert(handle == &DMA_handle_2);

    RX_channel_handler_call_count++;
}
