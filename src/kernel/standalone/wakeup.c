// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2018-2019 Mutant Industries ltd.
#include <test/kernel/standalone/wakeup.h>
#include <driver/interrupt.h>
#include <driver/stack.h>
#include <kernel.h>
#include <time.h>

#define __INIT_PROCESS_PRIORITY__       0
#define __TEST_PROCESS_1_PRIORITY__     1
#define __TEST_PROCESS_2_PRIORITY__     1
#define __IO_REINIT_ACTION_PRIORITY__   0xF000

#define BUTTON_PORT                     PORT_5
#define BUTTON_S1_PIN                   PIN_6

// -------------------------------------------------------------------------------------

static bool button_handler(IO_port_driver_t *, uint8_t interrupt_pin);
static signal_t test_process_1_entry_point(void);
static signal_t test_process_2_entry_point(Event_t *);
static void idle(void);

// -------------------------------------------------------------------------------------

__persistent static Process_control_block_t init_process_persistent = {0};
__persistent static Process_control_block_t test_process_1_persistent = {0};
__persistent static uint8_t test_stack_1_persistent[__TEST_STACK_SIZE__] = {0};
__persistent static Process_control_block_t test_process_2_persistent = {0};
__persistent static uint8_t test_stack_2_persistent[__TEST_STACK_SIZE__] = {0};

__persistent static Subscription_t button_press_subscription = {0};
__persistent static Event_t button_pressed_event = {0};
__persistent static Action_t IO_reinit_action = {0};

__persistent static IO_port_driver_t button_port_driver = {0};
__persistent static IO_pin_handle_t button_port_handle = {0};
__persistent static IO_port_driver_t led_port_driver = {0};

__persistent static volatile bool red_led_on = false;

// -------------------------------------------------------------------------------------

static void led_driver_reinit(IO_port_driver_t *_this) {
    IO_driver_reg_set(_this, DIR, RED_LED_PIN | GREEN_LED_PIN);
}

static void button_driver_reinit(IO_port_driver_t *_this) {
    IO_driver_reg_set(_this, OUT, BUTTON_S1_PIN);
    IO_driver_reg_set(_this, REN, BUTTON_S1_PIN);
    IO_driver_reg_set(_this, IES, BUTTON_S1_PIN);
}

// -------------------------------------------------------------------------------------

static void init() {
    // initialize test process 1
    zerofill(&test_process_1_persistent.create_config);

    test_process_1_persistent.create_config.priority = __TEST_PROCESS_1_PRIORITY__;
    test_process_1_persistent.create_config.stack_addr_low = (data_pointer_register_t) test_stack_1_persistent;
    test_process_1_persistent.create_config.stack_size = __TEST_STACK_SIZE__;
    test_process_1_persistent.create_config.entry_point = (process_entry_point_t) test_process_1_entry_point;

    // initialize test process 2
    zerofill(&test_process_2_persistent.create_config);

    test_process_2_persistent.create_config.priority = __TEST_PROCESS_2_PRIORITY__;
    test_process_2_persistent.create_config.stack_addr_low = (data_pointer_register_t) test_stack_2_persistent;
    test_process_2_persistent.create_config.stack_size = __TEST_STACK_SIZE__;
    test_process_2_persistent.create_config.entry_point = (process_entry_point_t) test_process_2_entry_point;
    test_process_2_persistent.create_config.arg_1 = &wakeup_event;

    // create process with higher priority than init process
    process_create(&test_process_1_persistent);
    // ...and schedule it
    process_schedule(&test_process_1_persistent, 0);

    // create process with higher priority than init process
    process_create(&test_process_2_persistent);
    // ...and schedule it
    process_schedule(&test_process_2_persistent, 0);

    // create action that shall reinitialize IO drivers on trigger
    action_create(&IO_reinit_action, NULL, IO_wakeup_reinit);
    // reinit IO with some high priority
    sorted_set_item_priority(&IO_reinit_action) = __IO_REINIT_ACTION_PRIORITY__;
    // ...and subscribe it to wakeup event
    event_subscribe(&wakeup_event, &IO_reinit_action);

    // LED port driver init
    IO_port_driver_create(&led_port_driver, LED_PORT, led_driver_reinit);
    // --- test persistent pin output state - green LED should never blink ---
    IO_driver_reg_set(&led_port_driver, OUT, GREEN_LED_PIN);
    // red LED off after reset
    IO_driver_reg_reset(&led_port_driver, OUT, RED_LED_PIN);

    red_led_on = false;

    IO_unlock();
}

void test_kernel_standalone_wakeup() {

    WDT_disable();

    // --- create and schedule process from current context, set wakeup on LPMx5 wakeup ---
    default_kernel_start(&init_process_persistent, __INIT_PROCESS_PRIORITY__, init, (bool) (SYSRSTIV & SYSRSTIV__LPM5WU));

    // switch to (suitable) low-power mode in idle
    idle();
}

// -------------------------------------------------------------------------------------

static void idle_status_register_reset(Process_control_block_t *idle) {
    // --- test do not enter low-power mode when scheduled ---
    deferred_stack_status_register_reset(idle->_stack_pointer, LPM4_bits);
}

static void idle() {
    Time_unit_t upcoming_event_time;

    // register pre-schedule hook that shall reset status register on deferred stack on init process
    process_current_pre_schedule_hook() = idle_status_register_reset;

    while (true) {

        interrupt_suspend();
        
        if ( ! get_upcoming_event_time(&upcoming_event_time)) {
            // suspend registered IO drivers
            IO_low_power_mode_prepare();
            // standard LPM4.5 enter
            LPM_x_5_enter();
        }

        // --- test pre-schedule hook - (reset LPM4_bits after init is scheduled and continue execution) ---
        interrupt_restore_with(LPM4_bits);
    }
}

// -------------------------------------------------------------------------------------

static signal_t test_process_1_entry_point() {

    // create event that shall be triggered on button press
    event_create(&button_pressed_event);
    // create subscription that shall blink with red LED when triggered
    subscription_create(&button_press_subscription, button_handler, &led_port_driver);
    // ...and subscribe it to button pressed event
    event_subscribe(&button_pressed_event, &button_press_subscription);

    // button port driver init
    IO_port_driver_create(&button_port_driver, BUTTON_PORT, button_driver_reinit);
    // init button port handle
    IO_port_handle_register(&button_port_driver, &button_port_handle, BUTTON_S1_PIN);
    // ...and register interrupt service to trigger button_pressed_event (argument 2 is the interrupt source pin)
    vector_register_handler(&button_port_handle, action(&button_pressed_event)->trigger, &button_pressed_event, NULL);

    // clear button interrupt flags to avoid erroneous port interrupts
    vector_clear_interrupt_flag(&button_port_handle);
    // enable button interrupts
    vector_set_enabled(&button_port_handle, true);

    // wait for button press subscription
    signal_wait();

    // unreachable
    return 0;
}

static bool button_handler(IO_port_driver_t *driver, uint8_t interrupt_pin) {
    assert(driver == &led_port_driver);
    assert(interrupt_pin == BUTTON_S1_PIN);

    if (red_led_on) {
        IO_driver_reg_set(driver, OUT, RED_LED_PIN);
    }
    else {
        IO_driver_reg_set(driver, OUT, RED_LED_PIN);
    }

    red_led_on = ! red_led_on;

    return true;
}

// -------------------------------------------------------------------------------------

static signal_t test_process_2_entry_point(Event_t *wakeup) {
    uint16_t i, millisecs;

    assert(wakeup == &wakeup_event);

    // --- test blocking wait on wakeup event ---
    while (event_wait(wakeup) == SYSTEM_WAKEUP_SIGNAL) {
        // --- test timing active after wakeup ---
        for (i = 0, millisecs = 250; i < 4; i++, millisecs >>= 1) {
            // 500ms, 250ms, 125ms and 62ms
            millisleep(millisecs);
            // red LED blink (4x state toggle)
            IO_driver_reg_toggle(&led_port_driver, OUT, RED_LED_PIN);
        }

        // --- test device shall enter LPM4.5 when no more timed events are scheduled ---
    }

    // unreachable
    return 0;
}
