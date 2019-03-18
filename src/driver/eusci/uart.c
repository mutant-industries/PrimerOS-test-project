// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/eusci/uart.h>
#include <driver/interrupt.h>

// -------------------------------------------------------------------------------------

#define __TEST_UART_NO__   2

static void on_character_received(UART_driver_t *owner, void *event_arg);
static void on_transmit_buffer_empty(UART_driver_t *owner, void *event_arg);
static void on_start_bit_received(UART_driver_t *owner, void *event_arg);
static void on_transmit_complete(UART_driver_t *owner, void *event_arg);

static volatile uint16_t on_character_received_call_count;
static volatile uint16_t on_transmit_buffer_empty_call_count;
static volatile uint16_t on_start_bit_received_call_count;
static volatile uint16_t on_transmit_complete_call_count;

static volatile uint8_t next_char;

// -------------------------------------------------------------------------------------

void test_driver_eusci_uart() {
    UART_baudrate_config_t baudrate_config;
    UART_transfer_config_t transfer_config;
    uint16_t uart_base;
    volatile uint16_t *reg_UCAxCTLW0, *reg_UCAxCTLW1, *reg_UCAxBRW, *reg_UCAxMCTLW, *reg_UCAxSTATW, *reg_UCAxIE;
#ifdef __UART_AUTO_BAUDRATE_CONTROL_ENABLE__
    volatile uint16_t *reg_UCAxABCTL;
#endif
#ifdef __UART_IrDA_CONTROL_ENABLE__
    volatile uint16_t *reg_UCAxIRCTL;
#endif

    WDT_disable();
    // DCO 8MHz, SMCLK input DCO / 1
    default_clock_setup(0, DCOFSEL_6, DIVM__1);

    uart_base = UART_BASE(__TEST_UART_NO__);

    reg_UCAxCTLW0 = (uint16_t *) (uart_base + OFS_UCAxCTLW0);
    reg_UCAxCTLW1 = (uint16_t *) (uart_base + OFS_UCAxCTLW1);
    reg_UCAxBRW = (uint16_t *) (uart_base + OFS_UCAxBRW);
    reg_UCAxMCTLW = (uint16_t *) (uart_base + OFS_UCAxMCTLW);
    reg_UCAxSTATW = (uint16_t *) (uart_base + OFS_UCAxSTATW);
#ifdef __UART_AUTO_BAUDRATE_CONTROL_ENABLE__
    reg_UCAxABCTL = (uint16_t *) (uart_base + OFS_UCAxABCTL);
#endif
#ifdef __UART_IrDA_CONTROL_ENABLE__
    reg_UCAxIRCTL = (uint16_t *) (uart_base + OFS_UCAxIRCTL);
#endif
    reg_UCAxIE = (uint16_t *) (uart_base + OFS_UCAxIE);

    // UART driver init
    UART_driver_register(&UART_driver_1, uart_base, UART_VECTOR(__TEST_UART_NO__));

    assert_not(UART_is_busy(&UART_driver_1));

    // --- test CTLW0 default values set ---

    // parity enable, even parity
    assert(*reg_UCAxCTLW0 & (UCPEN_1 | UCPAR__EVEN));
    assert(UART_control_reg(&UART_driver_1) & (UCPEN_1 | UCPAR__EVEN));
    // LSB first, 8-bit data, one stop bit
    assert(~(*reg_UCAxCTLW0) & (UCMSB_1 | UC7BIT__7BIT | UCSPB_1));
    assert(~UART_control_reg(&UART_driver_1) & (UCMSB_1 | UC7BIT__7BIT | UCSPB_1));

    // SMCLK (8MHz), baudrate 9600
    baudrate_config.clock_source = UCSSEL__SMCLK;
    baudrate_config.clock_prescaler = 52;
    baudrate_config.first_modulation_stage = 1;
    baudrate_config.second_modulation_stage = 0x49;
    baudrate_config.oversampling = true;

    // --- test baudrate config set ---
    UART_set_baudrate_config(&UART_driver_1, &baudrate_config);

    // clock source
    assert((*reg_UCAxCTLW0 & UCSSEL) == baudrate_config.clock_source);
    assert((UART_control_reg(&UART_driver_1) & UCSSEL) == baudrate_config.clock_source);
    // clock prescaler
    assert(*reg_UCAxBRW == baudrate_config.clock_prescaler);
    assert(UART_baudrate_control_reg(&UART_driver_1) == baudrate_config.clock_prescaler);
    // first modulation stage
    assert((((uint8_t) *reg_UCAxMCTLW) >> 4) == baudrate_config.first_modulation_stage);
    assert((((uint8_t) UART_modulation_control_reg(&UART_driver_1)) >> 4) == baudrate_config.first_modulation_stage);
    // second modulation stage
    assert((*reg_UCAxMCTLW >> 8) == baudrate_config.second_modulation_stage);
    assert((UART_modulation_control_reg(&UART_driver_1) >> 8) == baudrate_config.second_modulation_stage);
    // oversampling
    assert(((bool) (*reg_UCAxMCTLW & 1)) == baudrate_config.oversampling);
    assert(((bool) (UART_modulation_control_reg(&UART_driver_1) & UCOS16)) == baudrate_config.first_modulation_stage);

    // --- test mode set, persist config ---
    UART_set_transfer_config(&UART_driver_1, UCMODE_2, NULL);

    // parity enable, even parity
    assert(*reg_UCAxCTLW0 & (UCPEN_1 | UCPAR__EVEN));
    // LSB first, 8-bit data, one stop bit
    assert(~(*reg_UCAxCTLW0) & (UCMSB_1 | UC7BIT__7BIT | UCSPB_1));
    // mode change
    assert((*reg_UCAxCTLW0 & UCMODE) == UCMODE_2);

    // --- test mode and config set ---
    transfer_config.parity_enable = UCPEN_0;
    transfer_config.parity_select = UCPAR__ODD;
    transfer_config.receive_direction = UCMSB_1;
    transfer_config.character_length = UC7BIT__7BIT;
    transfer_config.stop_bit_select = UCSPB_1;

    UART_set_transfer_config(&UART_driver_1, UCMODE_0, &transfer_config);

    // parity disable, odd parity
    assert(~(*reg_UCAxCTLW0) & (UCPEN_1 | UCPAR__EVEN));
    // MSB first, 7-bit data, two stop bits
    assert(*reg_UCAxCTLW0 & (UCMSB_1 | UC7BIT__7BIT | UCSPB_1));
    // mode change
    assert((*reg_UCAxCTLW0 & UCMODE) == UCMODE_0);

    // --- test loopback set ---
    UART_set_loopback(&UART_driver_1, true);

    assert(*reg_UCAxSTATW & UCLISTEN_1);
    assert(UART_status_reg(&UART_driver_1) & UCLISTEN_1);

    UART_set_loopback(&UART_driver_1, false);

    assert(~(*reg_UCAxSTATW) & UCLISTEN_1);
    assert(~UART_status_reg(&UART_driver_1) & UCLISTEN_1);

    // -- test deglitch time set ---
    UART_deglitch_control_reg(&UART_driver_1) = UCGLIT_3;

    assert(*reg_UCAxCTLW1 == UCGLIT_3);
    assert(UART_deglitch_control_reg(&UART_driver_1) == UCGLIT_3);

    UART_deglitch_control_reg(&UART_driver_1) = UCGLIT_0;

    assert(*reg_UCAxCTLW1 == UCGLIT_0);
    assert(UART_deglitch_control_reg(&UART_driver_1) == UCGLIT_0);

#ifdef __UART_AUTO_BAUDRATE_CONTROL_ENABLE__
    // -- test auto baudrate detection enable toggle and delimiter set ---
    UART_set_auto_baudrate_detection(&UART_driver_1, true, UCDELIM_3);

    assert(*reg_UCAxABCTL & (UCABDEN | UCDELIM_3));
    assert(UART_auto_baudrate_control_reg(&UART_driver_1) & (UCABDEN | UCDELIM_3));

    UART_set_auto_baudrate_detection(&UART_driver_1, false, UCDELIM_3);

    assert(*reg_UCAxABCTL & UCDELIM_3);
    assert(UART_auto_baudrate_control_reg(&UART_driver_1) & UCDELIM_3);

    assert(~(*reg_UCAxABCTL) & UCABDEN);
    assert(~UART_auto_baudrate_control_reg(&UART_driver_1) & UCABDEN);

#endif
#ifdef __UART_IrDA_CONTROL_ENABLE__
    // --- test IrDA enable and config set ---
    UART_IrDA_config_t irda_config;

    irda_config.transmit_pulse_clock = UCIRTXCLK_1;
    irda_config.transmit_pulse_length = UCIRTXPL3;
    irda_config.receive_filter_enabled = UCIRRXFE_1;
    irda_config.receive_input_polarity = UCIRRXPL__LOW;
    irda_config.receive_filter_length = UCIRRXFL3;

    UART_set_IrDA_control(&UART_driver_1, true, &irda_config);

    assert(*reg_UCAxIRCTL & (UCIREN | UCIRTXCLK_1 | UCIRTXPL3 | UCIRRXFE_1 | UCIRRXPL__LOW | UCIRRXFL3));
    assert(UART_IrDA_control_reg(&UART_driver_1) & (UCIREN | UCIRTXCLK_1 | UCIRTXPL3 | UCIRRXFE_1 | UCIRRXPL__LOW | UCIRRXFL3));

    // --- test IrDA disable, persist config ---
    UART_set_IrDA_control(&UART_driver_1, false, NULL);

    assert(*reg_UCAxIRCTL & (UCIRTXCLK_1 | UCIRTXPL3 | UCIRRXFE_1 | UCIRRXPL__LOW | UCIRRXFL3));
    assert(UART_IrDA_control_reg(&UART_driver_1) & (UCIRTXCLK_1 | UCIRTXPL3 | UCIRRXFE_1 | UCIRRXPL__LOW | UCIRRXFL3));

    assert(~(*reg_UCAxIRCTL) & UCIREN);
    assert(~UART_IrDA_control_reg(&UART_driver_1) & UCIREN);
#endif

    // --- test interrupt handlers on loopback ---
    on_character_received_call_count = on_transmit_buffer_empty_call_count
            = on_start_bit_received_call_count = on_transmit_complete_call_count = 0;

    interrupt_enable();

    transfer_config.parity_enable = UCPEN_1;
    transfer_config.parity_select = UCPAR__EVEN;
    transfer_config.receive_direction = UCMSB_0;
    transfer_config.character_length = UC7BIT__8BIT;
    transfer_config.stop_bit_select = UCSPB_1;

    UART_set_transfer_config(&UART_driver_1, UCMODE_0, &transfer_config);
    UART_set_loopback(&UART_driver_1, true);

    UART_on_character_received(&UART_driver_1) = UART_event_handler(on_character_received);
    UART_on_transmit_buffer_empty(&UART_driver_1) = UART_event_handler(on_transmit_buffer_empty);
    UART_on_start_bit_received(&UART_driver_1) = UART_event_handler(on_start_bit_received);
    UART_on_transmit_complete(&UART_driver_1) = UART_event_handler(on_transmit_complete);
    UART_event_arg(&UART_driver_1) = &DMA_driver;

    // enable UART, transmit buffer empty interrupt flag should be set
    UART_reset_disable(&UART_driver_1);

    // -- test IE register setter ---
    UART_interrupt_enable(&UART_driver_1, UCRXIE | UCTXIE | UCSTTIE | UCTXCPTIE);

    assert(on_character_received_call_count == 0);
    assert(on_transmit_buffer_empty_call_count == 1);
    assert(on_start_bit_received_call_count == 0);
    assert(on_transmit_complete_call_count == 0);

    next_char = 0xAA;

    UART_TX_buffer(&UART_driver_1) = next_char;

    // spin-lock until byte sent
    while (UART_is_busy(&UART_driver_1));

    assert(on_character_received_call_count == 1);
    assert(on_transmit_buffer_empty_call_count == 2);
    assert(on_start_bit_received_call_count == 1);
    assert(on_transmit_complete_call_count == 1);

    // --- test interrupt enable is cleared when there is no handler ---
    UART_on_start_bit_received(&UART_driver_1) = NULL;

    next_char = 0x55;

    UART_TX_buffer(&UART_driver_1) = next_char;

    // spin-lock until byte sent
    while (UART_is_busy(&UART_driver_1));

    assert(on_character_received_call_count == 2);
    assert(on_transmit_buffer_empty_call_count == 3);
    assert(on_start_bit_received_call_count == 1);
    assert(on_transmit_complete_call_count == 2);

    assert(~(*reg_UCAxIE) & UCSTTIE);
    assert(~UART_IE_reg(&UART_driver_1) & UCSTTIE);

    // --- test driver dispose ---
    dispose(&UART_driver_1);

    UART_TX_buffer(&UART_driver_1) = next_char;

    // spin-lock until byte sent
    while (UART_is_busy(&UART_driver_1));

    assert(on_character_received_call_count == 2);
    assert(on_transmit_buffer_empty_call_count == 3);
    assert(on_start_bit_received_call_count == 1);
    assert(on_transmit_complete_call_count == 2);
}

// -------------------------------------------------------------------------------------

static void on_character_received(UART_driver_t *driver, void *event_arg) {
    assert(driver == &UART_driver_1);
    assert(event_arg == &DMA_driver);

    assert(UART_RX_buffer(driver) == next_char);

    on_character_received_call_count++;
}

static void on_transmit_buffer_empty(UART_driver_t *driver, void *event_arg) {
    assert(driver == &UART_driver_1);
    assert(event_arg == &DMA_driver);

    on_transmit_buffer_empty_call_count++;
}

static void on_start_bit_received(UART_driver_t *driver, void *event_arg) {
    assert(driver == &UART_driver_1);
    assert(event_arg == &DMA_driver);

    on_start_bit_received_call_count++;
}

static void on_transmit_complete(UART_driver_t *driver, void *event_arg) {
    assert(driver == &UART_driver_1);
    assert(event_arg == &DMA_driver);

    on_transmit_complete_call_count++;
}
