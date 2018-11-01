// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#define __WDT_DISABLE__

#include <test/driver/wdt/disabled.h>
#include <driver/wdt.h>


#define __TEST_INTERVAL__   64
#define __TEST_SSEL__       ACLK

// -------------------------------------------------------------------------------------

void test_driver_wdt_disabled() {

    volatile uint8_t wdtctl_0_original = (uint8_t) WDTCTL;
    volatile uint8_t wdtctl_0_current;

    WDT_hold();

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != (wdtctl_0_original | WDTHOLD)) {
        test_fail();
    }

    WDT_start();

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != (wdtctl_0_original | WDTHOLD)) {
        test_fail();
    }

    WDT_ssel(__TEST_SSEL__);

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != (wdtctl_0_original | WDTHOLD)) {
        test_fail();
    }

    WDT_clr_interval(__TEST_INTERVAL__);

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != (wdtctl_0_original | WDTHOLD)) {
        test_fail();
    }

    WDT_clr_ssel_interval(__TEST_SSEL__, __TEST_INTERVAL__);

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != (wdtctl_0_original | WDTHOLD)) {
        test_fail();
    }

    WDT_backup_clr_interval(__TEST_INTERVAL__);

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != (wdtctl_0_original | WDTHOLD)) {
        test_fail();
    }

    WDT_backup_clr_ssel_interval(__TEST_SSEL__, __TEST_INTERVAL__);

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != (wdtctl_0_original | WDTHOLD)) {
        test_fail();
    }

    WDT_restore();

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != (wdtctl_0_original | WDTHOLD)) {
        test_fail();
    }
}
