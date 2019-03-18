// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/eusci/spi.h>
#include <driver/interrupt.h>

// -------------------------------------------------------------------------------------

#define __TEST_SPI_1_NO__   2
#define __TEST_SPI_2_NO__   0

static void test_SPI_driver(SPI_driver_t *, uint16_t base, EUSCI_type, uint8_t vector_no);
static void on_character_received(SPI_driver_t *driver, void *event_arg);
static void on_transmit_buffer_empty(SPI_driver_t *driver, void *event_arg);

static volatile uint16_t on_character_received_call_count;
static volatile uint16_t on_transmit_buffer_empty_call_count;

static volatile uint8_t next_char;

// -------------------------------------------------------------------------------------

void test_driver_eusci_spi() {

    WDT_disable();
    // DCO 8MHz, SMCLK input DCO / 1
    default_clock_setup(0, DCOFSEL_6, DIVM__1);

    // --- test eUSCI_A SPI driver ---
    test_SPI_driver(&SPI_driver_1, SPI_A_BASE(__TEST_SPI_1_NO__), A, SPI_A_VECTOR(__TEST_SPI_1_NO__));
    // --- test eUSCI_B SPI driver ---
    test_SPI_driver(&SPI_driver_2, SPI_BASE(B, __TEST_SPI_1_NO__), B, SPI_VECTOR(B, __TEST_SPI_1_NO__));
    // --- test driver reuse ---
    test_SPI_driver(&SPI_driver_1, SPI_BASE(A, __TEST_SPI_2_NO__), A, SPI_VECTOR(A, __TEST_SPI_2_NO__));
}

static void test_SPI_driver(SPI_driver_t *driver, uint16_t base, EUSCI_type type, uint8_t vector_no) {
    SPI_bitrate_config_t bitrate_config;
    SPI_transfer_config_t transfer_config;
    volatile uint16_t *reg_UCxCTLW0, *reg_UCxBRW, *reg_UCxSTATW, *reg_UCxIE;

    reg_UCxCTLW0 = (uint16_t *) (base + (type == A ? OFS_UCAxCTLW0 : OFS_UCBxCTLW0));
    reg_UCxBRW = (uint16_t *) (base + (type == A ? OFS_UCAxBRW : OFS_UCBxBRW));
    reg_UCxSTATW = (uint16_t *) (base + (type == A ? OFS_UCAxSTATW : OFS_UCBxSTATW));
    reg_UCxIE = (uint16_t *) (base + (type == A ? OFS_UCAxIE : OFS_UCBxIE));

    SPI_driver_register(driver, base, type, vector_no);

    assert_not(SPI_is_busy(driver));

    bitrate_config.clock_source = UCSSEL__ACLK;
    bitrate_config.clock_prescaler = 0xAAAA;

    // --- test bitrate config set ---
    SPI_set_bitrate_config(driver, &bitrate_config);

    // clock source
    assert((*reg_UCxCTLW0 & UCSSEL) == bitrate_config.clock_source);
    assert((SPI_control_reg(driver) & UCSSEL) == bitrate_config.clock_source);
    // clock prescaler
    assert(*reg_UCxBRW == bitrate_config.clock_prescaler);
    assert(SPI_bitrate_control_reg(driver) == bitrate_config.clock_prescaler);

    bitrate_config.clock_source = UCSSEL__SMCLK;
    bitrate_config.clock_prescaler = 2;

    // --- test bitrate config update ---
    SPI_set_bitrate_config(driver, &bitrate_config);

    // clock source
    assert((*reg_UCxCTLW0 & UCSSEL) == bitrate_config.clock_source);
    assert((SPI_control_reg(driver) & UCSSEL) == bitrate_config.clock_source);
    // clock prescaler
    assert(*reg_UCxBRW == bitrate_config.clock_prescaler);
    assert(SPI_bitrate_control_reg(driver) == bitrate_config.clock_prescaler);

    // --- test mode and config set ---
    transfer_config.clock_phase = UCCKPH_1;
    transfer_config.cock_polarity = UCCKPL__HIGH;
    transfer_config.receive_direction = UCMSB_1;
    transfer_config.character_length = UC7BIT__7BIT;
    transfer_config.master_mode = UCMST_1;
    transfer_config.STE_mode = UCSTEM_1;

    SPI_set_transfer_config(driver, UCMODE_2, &transfer_config);

    assert(*reg_UCxCTLW0 & (UCCKPH_1 | UCCKPL__HIGH | UCMSB_1 | UC7BIT__7BIT | UCMST_1 | UCSTEM_1));
    // mode change
    assert((*reg_UCxCTLW0 & UCMODE) == UCMODE_2);

    // --- test mode set, persist config ---
    SPI_set_transfer_config(driver, UCMODE_0, NULL);

    assert(*reg_UCxCTLW0 & (UCCKPH_1 | UCCKPL__HIGH | UCMSB_1 | UC7BIT__7BIT | UCMST_1 | UCSTEM_1));
    // mode change
    assert((*reg_UCxCTLW0 & UCMODE) == UCMODE_0);

    // --- test loopback set ---
    SPI_set_loopback(driver, true);

    assert(*reg_UCxSTATW & UCLISTEN_1);
    assert(SPI_status_reg(driver) & UCLISTEN_1);

    SPI_set_loopback(driver, false);

    assert(~(*reg_UCxSTATW) & UCLISTEN_1);
    assert(~SPI_status_reg(driver) & UCLISTEN_1);

    // --- test interrupt handlers on loopback ---
    on_character_received_call_count = on_transmit_buffer_empty_call_count = 0;

    interrupt_enable();

    transfer_config.clock_phase = UCCKPH_1;
    transfer_config.cock_polarity = UCCKPL__LOW;
    transfer_config.receive_direction = UCMSB_1;
    transfer_config.character_length = UC7BIT__8BIT;
    // master mode for loopback test
    transfer_config.master_mode = UCMST_1;
    transfer_config.STE_mode = UCSTEM_0;

    SPI_set_transfer_config(driver, UCMODE_0, &transfer_config);
    SPI_set_loopback(driver, true);

    SPI_on_character_received(driver) = SPI_event_handler(on_character_received);
    SPI_on_transmit_buffer_empty(driver) = SPI_event_handler(on_transmit_buffer_empty);
    SPI_event_arg(driver) = &DMA_driver;

    // enable SPI, transmit buffer empty interrupt flag should be set
    SPI_reset_disable(driver);

    // -- test IE register setter ---
    SPI_interrupt_enable(driver, UCRXIE | UCTXIE);

    assert(on_character_received_call_count == 0);
    assert(on_transmit_buffer_empty_call_count == 1);

    next_char = 0xAA;

    SPI_TX_buffer(driver) = next_char;

    // spin-lock until byte sent
    while (SPI_is_busy(driver));

    assert(on_character_received_call_count == 1);
    assert(on_transmit_buffer_empty_call_count == 2);

    // --- test interrupt enable is cleared when there is no handler ---
    SPI_on_transmit_buffer_empty(driver) = NULL;

    next_char = 0x55;

    SPI_TX_buffer(driver) = next_char;

    // spin-lock until byte sent
    while (SPI_is_busy(driver));

    assert(on_character_received_call_count == 2);
    assert(on_transmit_buffer_empty_call_count == 2);

    assert(~(*reg_UCxIE) & UCTXIE);
    assert(~SPI_IE_reg(driver) & UCTXIE);

    // --- test driver dispose ---
    dispose(driver);

    SPI_TX_buffer(driver) = next_char;

    // spin-lock until byte sent
    while (SPI_is_busy(driver));

    assert(on_character_received_call_count == 2);
    assert(on_transmit_buffer_empty_call_count == 2);
}

// -------------------------------------------------------------------------------------

static void on_character_received(SPI_driver_t *driver, void *event_arg) {
    assert(event_arg == &DMA_driver);

    assert(SPI_RX_buffer(driver) == next_char);

    on_character_received_call_count++;
}

static void on_transmit_buffer_empty(SPI_driver_t *_, void *event_arg) {
    assert(event_arg == &DMA_driver);

    on_transmit_buffer_empty_call_count++;
}
