// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#define __WDT_INTERVAL_TIMER_MODE__

#include <test/driver/wdt/interval.h>
#include <driver/wdt.h>
#include <driver/vector.h>


#define __TEST_INTERVAL__   64
#define __TEST_SSEL__       SMCLK

// -------------------------------------------------------------------------------------

void driver_wdt_interval_test_ISR(void);
static volatile uint16_t test_ISR_call_count;

// -------------------------------------------------------------------------------------

void test_driver_wdt_interval() {

    test_ISR_call_count = 0;

    WDT_hold();
    default_clock_setup(0, DCOFSEL_0, DIVM__1);

    vector_handle_register(&vector, NULL, WDT_VECTOR, (uint16_t) &SFRIE1, WDTIE, (uint16_t) &SFRIFG1, WDTIFG);

    vector_register_raw_handler(&vector, driver_wdt_interval_test_ISR, true);
    vector_clear_interrupt_flag(&vector);
    vector_set_enabled(&vector, true);

    __enable_interrupt();

    WDT_clr_ssel_interval(__TEST_SSEL__, __TEST_INTERVAL__);
    // delay cycles does not do exactly what it claims - for __delay_cycles(64) just 40 cycles pass
    __delay_cycles(__TEST_INTERVAL__ * 2);

    assert(test_ISR_call_count);

    WDT_hold(); __delay_cycles(6);    // hold and make sure IFG is not set next instruction

    test_ISR_call_count = 0;

    __delay_cycles(__TEST_INTERVAL__ * 2);

    assert( ! test_ISR_call_count);

    // --- pause / restore ---

    WDT_clr_interval(__TEST_INTERVAL__);

    {
        WDT_backup_hold();

        __delay_cycles(__TEST_INTERVAL__ * 2);

        assert( ! test_ISR_call_count);

        WDT_restore();

        __delay_cycles(__TEST_INTERVAL__ * 2);

        assert(test_ISR_call_count);
    }

    WDT_hold(); __delay_cycles(6);    // hold and make sure IFG is not set next instruction

    test_ISR_call_count = 0;

    // --- pause / restore when not started ---

    {
        WDT_backup_hold();

        __delay_cycles(__TEST_INTERVAL__ * 2);

        assert( ! test_ISR_call_count);

        WDT_restore();

        __delay_cycles(__TEST_INTERVAL__ * 2);

        assert( ! test_ISR_call_count);
    }

    WDT_hold(); __delay_cycles(6);    // hold and make sure IFG is not set next instruction

    test_ISR_call_count = 0;

    // --- pause / change state / restore ---

    WDT_clr_interval(__TEST_INTERVAL__);

    {
        WDT_backup_clr_interval(__TEST_INTERVAL__);

        __delay_cycles(__TEST_INTERVAL__ * 2);

        assert(test_ISR_call_count);

        WDT_clr_restore();

        test_ISR_call_count = 0;

        __delay_cycles(__TEST_INTERVAL__ * 2);

        assert(test_ISR_call_count);
    }

    WDT_hold(); __delay_cycles(6);    // hold and make sure IFG is not set next instruction

    test_ISR_call_count = 0;

    // --- pause / change state / restore when not started ---

    {
        WDT_backup_clr_interval(__TEST_INTERVAL__);

        __delay_cycles(__TEST_INTERVAL__ * 2);

        assert(test_ISR_call_count);

        WDT_clr_restore();

        test_ISR_call_count = 0;

        __delay_cycles(__TEST_INTERVAL__ * 2);

        assert( ! test_ISR_call_count);
    }

    dispose(&vector);
}

void __interrupt driver_wdt_interval_test_ISR() {
    test_ISR_call_count++;
    // WDT interrupt flag is cleared by hardware
}
