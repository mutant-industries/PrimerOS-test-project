// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/driver/wdt/watchdog.h>
#include <driver/wdt.h>


#define __TEST_DEFAULT_INTERVAL__   512
#define __TEST_DEFAULT_SSEL__       VLOCLK

#define __TEST_INTERVAL__           64
#define __TEST_SSEL__               ACLK

#define expand_ssel(param) \
    __WDT_param_expand__(WDTSSEL__, param)

#define expand_interval(param) \
    __WDT_param_expand__(WDTIS__, param)

// -------------------------------------------------------------------------------------

void test_driver_wdt_watchdog() {

    default_clock_setup();

    volatile uint8_t wdtctl_0_original = (uint8_t) WDTCTL;
    volatile uint8_t  wdtctl_0_current;

    WDT_hold();

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != (wdtctl_0_original | WDTHOLD)) {
        test_fail();
    }

    WDT_start();

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != (wdtctl_0_original & ~WDTHOLD)) {
        test_fail();
    }

    WDT_clr();

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != (wdtctl_0_original & ~WDTHOLD)) {
        test_fail();
    }

    WDT_ssel(__TEST_SSEL__);

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != ((wdtctl_0_original & ~WDTHOLD  & ~WDTSSEL) | expand_ssel(__TEST_SSEL__))) {
        test_fail();
    }

    WDT_clr_interval(__TEST_INTERVAL__);

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != ((wdtctl_0_original & ~WDTHOLD  & ~WDTSSEL & ~WDTIS)
            | expand_ssel(__TEST_SSEL__) | expand_interval(__TEST_INTERVAL__))) {
        test_fail();
    }

    WDT_clr_ssel_interval(__TEST_DEFAULT_SSEL__, __TEST_DEFAULT_INTERVAL__);

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != ((wdtctl_0_original & ~WDTHOLD  & ~WDTSSEL & ~WDTIS)
            | expand_ssel(__TEST_DEFAULT_SSEL__) | expand_interval(__TEST_DEFAULT_INTERVAL__))) {
        test_fail();
    }

    WDT_hold();

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != ((wdtctl_0_original & ~WDTSSEL & ~WDTIS) | WDTHOLD
            | expand_ssel(__TEST_DEFAULT_SSEL__) | expand_interval(__TEST_DEFAULT_INTERVAL__))) {
        test_fail();
    }

    WDT_clr_ssel_interval(__TEST_SSEL__, __TEST_INTERVAL__);

    wdtctl_0_current = (uint8_t) WDTCTL;

    if (wdtctl_0_current != ((wdtctl_0_original & ~WDTHOLD  & ~WDTSSEL & ~WDTIS)
            | expand_ssel(__TEST_SSEL__) | expand_interval(__TEST_INTERVAL__))) {
        test_fail();
    }

    // --- pause / restore ---

    WDT_start();

    {
        WDT_backup_hold();

        wdtctl_0_current = (uint8_t) WDTCTL;

        if (wdtctl_0_current != ((wdtctl_0_original & ~WDTSSEL & ~WDTIS) | WDTHOLD
                | expand_ssel(__TEST_SSEL__) | expand_interval(__TEST_INTERVAL__))) {
            test_fail();
        }

        WDT_restore();

        wdtctl_0_current = (uint8_t) WDTCTL;

        if (wdtctl_0_current != ((wdtctl_0_original & ~WDTHOLD & ~WDTSSEL & ~WDTIS)
                | expand_ssel(__TEST_SSEL__) | expand_interval(__TEST_INTERVAL__))) {
            test_fail();
        }
    }

    // --- pause / restore when not started ---

    WDT_hold();

    {
         WDT_backup_hold();

         wdtctl_0_current = (uint8_t) WDTCTL;

         if (wdtctl_0_current != ((wdtctl_0_original & ~WDTSSEL & ~WDTIS) | WDTHOLD
                 | expand_ssel(__TEST_SSEL__) | expand_interval(__TEST_INTERVAL__))) {
             test_fail();
         }

         WDT_restore();

         wdtctl_0_current = (uint8_t) WDTCTL;

         if (wdtctl_0_current != ((wdtctl_0_original & ~WDTSSEL & ~WDTIS) | WDTHOLD
                 | expand_ssel(__TEST_SSEL__) | expand_interval(__TEST_INTERVAL__))) {
             test_fail();
         }
     }

    // --- pause / change state / restore ---

    WDT_start();

    {
        WDT_backup_clr_interval(__TEST_DEFAULT_INTERVAL__);

        wdtctl_0_current = (uint8_t) WDTCTL;

        if (wdtctl_0_current != ((wdtctl_0_original & ~WDTHOLD & ~WDTSSEL & ~WDTIS)
                | expand_ssel(__TEST_SSEL__) | expand_interval(__TEST_DEFAULT_INTERVAL__))) {
            test_fail();
        }

        WDT_clr_restore();

        wdtctl_0_current = (uint8_t) WDTCTL;

        if (wdtctl_0_current != ((wdtctl_0_original & ~WDTHOLD & ~WDTSSEL & ~WDTIS)
                | expand_ssel(__TEST_SSEL__) | expand_interval(__TEST_INTERVAL__))) {
            test_fail();
        }
    }

    // --- pause / change state / restore when not started ---

    WDT_hold();

    {
        WDT_backup_clr_ssel_interval(__TEST_DEFAULT_SSEL__, __TEST_DEFAULT_INTERVAL__);

        wdtctl_0_current = (uint8_t) WDTCTL;

        if (wdtctl_0_current != ((wdtctl_0_original & ~WDTHOLD & ~WDTSSEL & ~WDTIS)
                | expand_ssel(__TEST_DEFAULT_SSEL__) | expand_interval(__TEST_DEFAULT_INTERVAL__))) {
            test_fail();
        }

        WDT_clr_restore();

        wdtctl_0_current = (uint8_t) WDTCTL;

        if (wdtctl_0_current != ((wdtctl_0_original & ~WDTSSEL & ~WDTIS) | WDTHOLD
                | expand_ssel(__TEST_SSEL__) | expand_interval(__TEST_INTERVAL__))) {
            test_fail();
        }
    }

    WDT_clr_ssel_interval(__TEST_SSEL__, __TEST_INTERVAL__);

    expect_reset();
}
