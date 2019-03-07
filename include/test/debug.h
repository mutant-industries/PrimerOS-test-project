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
#define IO_debug_unlock() \
    PM5CTL0 &= ~LOCKLPM5;

/**
 * - LED pins direction out, low
 * - debug pins direction out, low
 */
#define IO_debug_init() \
    _PORT_REG_OUT(LED_PORT) &= ~(RED_LED_PIN | GREEN_LED_PIN); \
    _PORT_REG_DIR(LED_PORT) |= (RED_LED_PIN | GREEN_LED_PIN); \
    _PORT_REG_OUT(DEBUG_PORT) &= ~(DEBUG_PIN_1 | DEBUG_PIN_2 | DEBUG_PIN_3 | DEBUG_PIN_4 | DEBUG_PIN_5 | DEBUG_PIN_6); \
    _PORT_REG_DIR(DEBUG_PORT) |= DEBUG_PIN_1 | DEBUG_PIN_2 | DEBUG_PIN_3 | DEBUG_PIN_4 | DEBUG_PIN_5 | DEBUG_PIN_6;

#define IO_green_led_on() \
        _PORT_REG_OUT(LED_PORT) |= GREEN_LED_PIN;

#define IO_green_led_off() \
        _PORT_REG_OUT(LED_PORT) &= ~GREEN_LED_PIN;

#define IO_green_led_toggle() \
        _PORT_REG_OUT(LED_PORT) ^= GREEN_LED_PIN;


#define IO_red_led_on() \
        _PORT_REG_OUT(LED_PORT) |= RED_LED_PIN;

#define IO_red_led_off() \
        _PORT_REG_OUT(LED_PORT) &= ~RED_LED_PIN;

#define IO_red_led_toggle() \
        _PORT_REG_OUT(LED_PORT) ^= RED_LED_PIN;


#define IO_debug_pin_1_set() \
        _PORT_REG_OUT(DEBUG_PORT) |= DEBUG_PIN_1;

#define IO_debug_pin_1_reset() \
        _PORT_REG_OUT(DEBUG_PORT) &= ~DEBUG_PIN_1;

#define IO_debug_pin_1_toggle() \
        _PORT_REG_OUT(DEBUG_PORT) ^= DEBUG_PIN_1;

#define IO_debug_pin_2_set() \
        _PORT_REG_OUT(DEBUG_PORT) |= DEBUG_PIN_2;

#define IO_debug_pin_2_reset() \
        _PORT_REG_OUT(DEBUG_PORT) &= ~DEBUG_PIN_2;

#define IO_debug_pin_2_toggle() \
        _PORT_REG_OUT(DEBUG_PORT) ^= DEBUG_PIN_2;

#define IO_debug_pin_3_set() \
        _PORT_REG_OUT(DEBUG_PORT) |= DEBUG_PIN_3;

#define IO_debug_pin_3_reset() \
        _PORT_REG_OUT(DEBUG_PORT) &= ~DEBUG_PIN_3;

#define IO_debug_pin_3_toggle() \
        _PORT_REG_OUT(DEBUG_PORT) ^= DEBUG_PIN_3;

#define IO_debug_pin_4_set() \
        _PORT_REG_OUT(DEBUG_PORT) |= DEBUG_PIN_4;

#define IO_debug_pin_4_reset() \
        _PORT_REG_OUT(DEBUG_PORT) &= ~DEBUG_PIN_4;

#define IO_debug_pin_4_toggle() \
        _PORT_REG_OUT(DEBUG_PORT) ^= DEBUG_PIN_4;

#define IO_debug_pin_5_set() \
        _PORT_REG_OUT(DEBUG_PORT) |= DEBUG_PIN_5;

#define IO_debug_pin_5_reset() \
        _PORT_REG_OUT(DEBUG_PORT) &= ~DEBUG_PIN_5;

#define IO_debug_pin_5_toggle() \
        _PORT_REG_OUT(DEBUG_PORT) ^= DEBUG_PIN_5;

#define IO_debug_pin_6_set() \
        _PORT_REG_OUT(DEBUG_PORT) |= DEBUG_PIN_6;

#define IO_debug_pin_6_reset() \
        _PORT_REG_OUT(DEBUG_PORT) &= ~DEBUG_PIN_6;

#define IO_debug_pin_6_toggle() \
        _PORT_REG_OUT(DEBUG_PORT) ^= DEBUG_PIN_6;

// --------------------------------------------------------------------------------------

// port base address
#define _PORT_BASE(NO)           ((uint8_t *) __PORT_BASE_EX__(NO))
// ! following works only for odd port number !
// input
#define _PORT_REG_OUT(NO)        *((volatile uint8_t *) _PORT_BASE(NO) + OFS_PAOUT)
// direction: 0b = input, 1b = output
#define _PORT_REG_DIR(NO)        *((volatile uint8_t *) _PORT_BASE(NO) + OFS_PADIR)

// concatenation of expanded parameter
#define __PORT_BASE_EX__(NO)      P## NO ##_BASE

// --------------------------------------------------------------------------------------


#endif /* _TEST_DEBUG_H_ */
