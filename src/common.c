// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/common.h>
#include <msp430.h>

// -------------------------------------------------------------------------------------

extern volatile uint16_t last_passed_test_id;

// -------------------------------------------------------------------------------------

void test_fail() {
    volatile uint16_t i;

    WDT_disable();
    default_clock_setup(0, DCOFSEL_0, DIVM__1);
    IO_debug_unlock();

    while (1) {
        IO_red_led_toggle();

        for (i = 50000; i > 0; i--);
    }
}

void test_pass() {
    volatile uint16_t i;

    WDT_disable();
    default_clock_setup(0, DCOFSEL_0, DIVM__1);
    IO_debug_unlock();

    IO_red_led_off();
    IO_green_led_on();

    for (i = 1000; i > 0; i--);

    IO_green_led_off();
}

#ifdef __TI_COMPILER_VERSION__
#pragma FUNCTION_OPTIONS(expect_reset, "-O1")   // with -O2 and more the last_passed_test_id++ is optimized out
#endif
void expect_reset(void) {
    volatile uint16_t i;

    last_passed_test_id++;

    IO_debug_unlock();
    IO_green_led_on();
    IO_red_led_off();

    while (1) {
        for (i = 5000; i > 0; i--);

        IO_red_led_toggle();
        IO_green_led_toggle();
    }
}

// -------------------------------------------------------------------------------------

void WDT_disable() {
    WDTCTL = WDTPW | WDTHOLD | (WDTCTL & (WDTSSEL | WDTTMSEL | WDTIS));
}

void default_clock_setup(uint16_t DCO_range_select, uint16_t DCO_frequency_select, uint16_t SMCLK_divider) {

    volatile uint16_t clockSource;
    volatile uint16_t clockSourceDivider;
    volatile uint16_t temp;

    // -------------------------------------------------------------------------
    // Set DCO requested frequency
    uint16_t dcorsel = DCO_range_select;
    uint16_t dcofsel = DCO_frequency_select;

    uint16_t tempCSCTL3 = 0;
    // Unlock CS control register
    CSCTL0 = CSKEY;

    // Assuming SMCLK and MCLK are sourced from DCO
    // Store CSCTL3 settings to recover later
    tempCSCTL3 = CSCTL3;

    // Keep overshoot transient within specification by setting clk
    // sources to divide by 4
    // Clear the DIVS & DIVM masks (~0x77) and set both fields to 4 divider
    CSCTL3 = (CSCTL3 & (~(0x77))) | DIVS1 | DIVM1;

    // Set user's frequency selection for DCO
    CSCTL1 = (dcorsel + dcofsel);

    // Delay by ~10us to let DCO settle. cycles to wait = 20 cycles buffer + (10us * (x MHz/4))
    __delay_cycles(180);

    // Set all dividers
    CSCTL3 = tempCSCTL3;

    // -------------------------------------------------------------------------
    // Set MCLK = DCO with frequency divider of 1
    clockSource = SELM__DCOCLK;
    clockSourceDivider = DIVM__1;

    temp = CSCTL3;

    CSCTL2 &= ~(SELM_7);
    CSCTL2 |= clockSource;
    CSCTL3 = (temp & ~(DIVM0 | DIVM1 | DIVM2)) | clockSourceDivider;

    // -------------------------------------------------------------------------
    // Set SMCLK = DCO with frequency divider of 1
    clockSource = SELM__DCOCLK;
    clockSourceDivider = SMCLK_divider;

    temp = CSCTL3;

    clockSource = clockSource << 4;
    clockSourceDivider = clockSourceDivider << 4;

    CSCTL2 &= ~(SELS_7);
    CSCTL2 |= clockSource;
    CSCTL3 = (temp & ~(DIVS0 | DIVS1 | DIVS2)) | clockSourceDivider;

    // -------------------------------------------------------------------------
    // Set ACLK = LFMODOSC with frequency divider of 1
    clockSource = SELM__LFMODOSC;
    clockSourceDivider = DIVM__1;

    temp = CSCTL3;

    clockSource = clockSource << 8;
    clockSourceDivider = clockSourceDivider << 8;

    CSCTL2 &= ~(SELA_7);
    CSCTL2 |= clockSource;
    CSCTL3 = (temp & ~(DIVA0 | DIVA1 | DIVA2)) | clockSourceDivider;

    // Lock CS control register
    CSCTL0_H = 0x00;
}

void power_on_reset() {
    PMMCTL0_H = PMMPW_H;
    PMMCTL0 |= PMMSWPOR;
    PMMCTL0_H = 0x00;
}

void LPM_x_5_enter() {

    WDT_disable();

    _disable_interrupts();

    *((volatile uint8_t *)((uint16_t) (PMM_BASE + OFS_PMMCTL0_H))) = PMMPW_H;
    *((volatile uint8_t *)((uint16_t) (PMM_BASE + OFS_PMMCTL0))) |= PMMREGOFF;
    *((volatile uint8_t *)((uint16_t) (PMM_BASE + OFS_PMMCTL0_L))) &= ~SVSHE;
    *((volatile uint8_t *)((uint16_t) (PMM_BASE + OFS_PMMCTL0_H))) = 0x00;

    LPM4;
}
