/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *  MSP430 IO helper macros
 *
 *  Copyright (c) 2018-2019 Mutant Industries ltd.
 */

#ifndef _TEST_DEBUG_H_
#define _TEST_DEBUG_H_

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// --------------------------------------------------------------------------------------

#define PORT_P1     1
#define PORT_P2     2
#define PORT_P3     3
#define PORT_P4     4
#define PORT_P5     5
#define PORT_P6     6
#define PORT_P7     7
#define PORT_P8     8
#define PORT_P9     9
#define PORT_P10    10
#define PORT_P11    11
#define PORT_PA     1
#define PORT_PB     3
#define PORT_PC     5
#define PORT_PD     7
#define PORT_PE     9
#define PORT_PF     11
#define PORT_PJ     13

#define PIN0        (0x0001)
#define PIN1        (0x0002)
#define PIN2        (0x0004)
#define PIN3        (0x0008)
#define PIN4        (0x0010)
#define PIN5        (0x0020)
#define PIN6        (0x0040)
#define PIN7        (0x0080)
#define PIN8        (0x0100)
#define PIN9        (0x0200)
#define PIN10       (0x0400)
#define PIN11       (0x0800)
#define PIN12       (0x1000)
#define PIN13       (0x2000)
#define PIN14       (0x4000)
#define PIN15       (0x8000)

// --------------------------------------------------------------------------------------

/**
 * LEDs
 */
#define LED_PORT                        PORT_P1
#define RED_LED_PIN                     PIN0
#define GREEN_LED_PIN                   PIN1

/**
 * Buttons
 */
#define BUTTON_PORT                     PORT_P5
#define BUTTON_S1_PIN                   PIN6
#define BUTTON_S2_PIN                   PIN5

/**
 * Debug signal ports
 */
#define DEBUG_PORT                      PORT_P3
#define DEBUG_PIN_1                     PIN0
#define DEBUG_PIN_2                     PIN1
#define DEBUG_PIN_3                     PIN2
#define DEBUG_PIN_4                     PIN3
#define DEBUG_PIN_5                     PIN4
#define DEBUG_PIN_6                     PIN5

// --------------------------------------------------------------------------------------

/**
 * Disable the GPIO power-on default high-impedance mode
 */
#define IO_unlock() \
    PM5CTL0 &= ~LOCKLPM5;

/**
 * - LED pins direction out, low
 * - debug pins direction out, low
 * - button pins direction in, pull-up resistor, interrupt edge high-to-low
 */
#define IO_debug_init() \
    PORT_REG_OUT(LED_PORT) &= ~(RED_LED_PIN | GREEN_LED_PIN); \
    PORT_REG_DIR(LED_PORT) |= (RED_LED_PIN | GREEN_LED_PIN); \
    PORT_REG_OUT(DEBUG_PORT) &= ~(DEBUG_PIN_1 | DEBUG_PIN_2 | DEBUG_PIN_3 | DEBUG_PIN_4 | DEBUG_PIN_5 | DEBUG_PIN_6); \
    PORT_REG_DIR(DEBUG_PORT) |= DEBUG_PIN_1 | DEBUG_PIN_2 | DEBUG_PIN_3 | DEBUG_PIN_4 | DEBUG_PIN_5 | DEBUG_PIN_6;

#define IO_green_led_on() \
        PORT_REG_OUT(LED_PORT) |= GREEN_LED_PIN;

#define IO_green_led_off() \
        PORT_REG_OUT(LED_PORT) &= ~GREEN_LED_PIN;

#define IO_green_led_toggle() \
        PORT_REG_OUT(LED_PORT) ^= GREEN_LED_PIN;


#define IO_red_led_on() \
        PORT_REG_OUT(LED_PORT) |= RED_LED_PIN;

#define IO_red_led_off() \
        PORT_REG_OUT(LED_PORT) &= ~RED_LED_PIN;

#define IO_red_led_toggle() \
        PORT_REG_OUT(LED_PORT) ^= RED_LED_PIN;


#define IO_debug_pin_1_set() \
        PORT_REG_OUT(DEBUG_PORT) |= DEBUG_PIN_1;

#define IO_debug_pin_1_reset() \
        PORT_REG_OUT(DEBUG_PORT) &= ~DEBUG_PIN_1;

#define IO_debug_pin_1_toggle() \
        PORT_REG_OUT(DEBUG_PORT) ^= DEBUG_PIN_1;

#define IO_debug_pin_2_set() \
        PORT_REG_OUT(DEBUG_PORT) |= DEBUG_PIN_2;

#define IO_debug_pin_2_reset() \
        PORT_REG_OUT(DEBUG_PORT) &= ~DEBUG_PIN_2;

#define IO_debug_pin_2_toggle() \
        PORT_REG_OUT(DEBUG_PORT) ^= DEBUG_PIN_2;

#define IO_debug_pin_3_set() \
        PORT_REG_OUT(DEBUG_PORT) |= DEBUG_PIN_3;

#define IO_debug_pin_3_reset() \
        PORT_REG_OUT(DEBUG_PORT) &= ~DEBUG_PIN_3;

#define IO_debug_pin_3_toggle() \
        PORT_REG_OUT(DEBUG_PORT) ^= DEBUG_PIN_3;

#define IO_debug_pin_4_set() \
        PORT_REG_OUT(DEBUG_PORT) |= DEBUG_PIN_4;

#define IO_debug_pin_4_reset() \
        PORT_REG_OUT(DEBUG_PORT) &= ~DEBUG_PIN_4;

#define IO_debug_pin_4_toggle() \
        PORT_REG_OUT(DEBUG_PORT) ^= DEBUG_PIN_4;

#define IO_debug_pin_5_set() \
        PORT_REG_OUT(DEBUG_PORT) |= DEBUG_PIN_5;

#define IO_debug_pin_5_reset() \
        PORT_REG_OUT(DEBUG_PORT) &= ~DEBUG_PIN_5;

#define IO_debug_pin_5_toggle() \
        PORT_REG_OUT(DEBUG_PORT) ^= DEBUG_PIN_5;

#define IO_debug_pin_6_set() \
        PORT_REG_OUT(DEBUG_PORT) |= DEBUG_PIN_6;

#define IO_debug_pin_6_reset() \
        PORT_REG_OUT(DEBUG_PORT) &= ~DEBUG_PIN_6;

#define IO_debug_pin_6_toggle() \
        PORT_REG_OUT(DEBUG_PORT) ^= DEBUG_PIN_6;

// --------------------------------------------------------------------------------------

// port base address
#define PORT_BASE(NO)           ((uint8_t *) _PORT_BASE_EX_(NO))
// input
#define PORT_REG_IN(NO)         *((volatile uint8_t *) PORT_BASE(NO) + OFS_PAIN)
// output when in output mode, input mode: 0b = pulldown selected, 1b = pullup selected
#define PORT_REG_OUT(NO)        *((volatile uint8_t *) PORT_BASE(NO) + OFS_PAOUT)
// direction: 0b = input, 1b = output
#define PORT_REG_DIR(NO)        *((volatile uint8_t *) PORT_BASE(NO) + OFS_PADIR)
// resistor enable
#define PORT_REG_REN(NO)        *((volatile uint8_t *) PORT_BASE(NO) + OFS_PAREN)
// function select 0
#define PORT_REG_SEL0(NO)       *((volatile uint8_t *) PORT_BASE(NO) + OFS_PASEL0)
// function select 1
#define PORT_REG_SEL1(NO)       *((volatile uint8_t *) PORT_BASE(NO) + OFS_PASEL1)
// interrupt vector
#define PORT_REG_IV(NO)         *((volatile uint8_t *) DIO_BASE + _OFS_IV_EX_(NO))
// complement selection
#define PORT_REG_SELC(NO)       *((volatile uint8_t *) PORT_BASE(NO) + OFS_PASELC)
// interrupt edge select: 0b = low-to-high, 1b = high-to-low
#define PORT_REG_IES(NO)        *((volatile uint8_t *) PORT_BASE(NO) + OFS_PAIES)
// interrupt enable
#define PORT_REG_IE(NO)         *((volatile uint8_t *) PORT_BASE(NO) + OFS_PAIE)
// interrupt flags
#define PORT_REG_IFG(NO)        *((volatile uint8_t *) PORT_BASE(NO) + OFS_PAIFG)


// concatenation of expanded parameter
#define _PORT_BASE_EX_(NO)      P## NO ##_BASE
#define _OFS_IV_EX_(NO)         OFS_P## NO ##IV

// --------------------------------------------------------------------------------------


#endif /* _TEST_DEBUG_H_ */
