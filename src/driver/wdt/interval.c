// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#define __WDT_INTERVAL_TIMER_MODE__

#include <test/driver/wdt/interval.h>
#include <driver/wdt.h>
#include <driver/vector.h>


#define __TEST_INTERVAL__   64
#define __TEST_SSEL__       SMCLK

// -------------------------------------------------------------------------------------

static void _test_ISR(void);
static volatile uint16_t _test_ISR_call_count;

Vector_handle_t h;

// -------------------------------------------------------------------------------------

void test_driver_wdt_interval() {

    _test_ISR_call_count = 0;

    __WDT_hold();
    default_clock_setup();

    vector_handle_register(&h, NULL, WDT_VECTOR, (uint16_t) &SFRIE1, WDTIE, (uint16_t) &SFRIFG1, WDTIFG);

    h.register_handler(&h, (void (*)(void)) _test_ISR, true);
    h.clear_interrupt_flag(&h);
    h.set_enabled(&h, true);

    __enable_interrupt();

    __WDT_clr_ssel_interval(__TEST_SSEL__, __TEST_INTERVAL__);
    // delay cycles does not do exactly what it claims - for __delay_cycles(64) just 40 cycles pass
    __delay_cycles(__TEST_INTERVAL__ * 2);

    if ( ! _test_ISR_call_count) {
        test_fail();
    }

    __WDT_hold();

    _test_ISR_call_count = 0;

    __delay_cycles(__TEST_INTERVAL__ * 2);

    if (_test_ISR_call_count) {
        test_fail();
    }

    // --- pause / restore ---

    __WDT_clr_interval(__TEST_INTERVAL__);

    {
        __WDT_backup_hold();

        __delay_cycles(__TEST_INTERVAL__ * 2);

        if (_test_ISR_call_count) {
            test_fail();
        }

        __WDT_restore();

        __delay_cycles(__TEST_INTERVAL__ * 2);

        if ( ! _test_ISR_call_count) {
            test_fail();
        }
    }

    __WDT_hold(); __delay_cycles(6);    // hold and make sure IFG is not set next instruction

    _test_ISR_call_count = 0;

    // --- pause / restore when not started ---

    {
        __WDT_backup_hold();

        __delay_cycles(__TEST_INTERVAL__ * 2);

        if (_test_ISR_call_count) {
            test_fail();
        }

        __WDT_restore();

        __delay_cycles(__TEST_INTERVAL__ * 2);

        if (_test_ISR_call_count) {
            test_fail();
        }
    }

    __WDT_hold(); __delay_cycles(6);    // hold and make sure IFG is not set next instruction

    _test_ISR_call_count = 0;

    // --- pause / change state / restore ---

    __WDT_clr_interval(__TEST_INTERVAL__);

    {
        __WDT_backup_clr_interval(__TEST_INTERVAL__);

        __delay_cycles(__TEST_INTERVAL__ * 2);

        if ( ! _test_ISR_call_count) {
            test_fail();
        }

        __WDT_clr_restore();

        _test_ISR_call_count = 0;

        __delay_cycles(__TEST_INTERVAL__ * 2);

        if ( ! _test_ISR_call_count) {
            test_fail();
        }
    }

    __WDT_hold(); __delay_cycles(6);    // hold and make sure IFG is not set next instruction

    _test_ISR_call_count = 0;

    // --- pause / change state / restore when not started ---

    {
        __WDT_backup_clr_interval(__TEST_INTERVAL__);

        __delay_cycles(__TEST_INTERVAL__ * 2);

        if ( ! _test_ISR_call_count) {
            test_fail();
        }

        __WDT_clr_restore();

        _test_ISR_call_count = 0;

        __delay_cycles(__TEST_INTERVAL__ * 2);

        if (_test_ISR_call_count) {
            test_fail();
        }
    }

    dispose(&h);
}

static __interrupt void _test_ISR() {
    _test_ISR_call_count++;
    // WDT interrupt flag is cleared by hardware
}
