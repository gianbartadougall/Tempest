/**
 * @file button.c
 * @author Gian Barta-Dougall
 * @brief System file for button
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "button.h"
#include "utilities.h"
#include <assert.h>

/* Private Includes */
#include "button_configuration.h"

/* Private macros */
#define PORT(button)                  (buttons[button].port)
#define IDR(button)                   (PORT(button)->IDR)
#define PIN(button)                   (buttons[button].pin)
#define BUTTON_IS_RESET(button)       (buttons[button].reset == 1)
#define ACTIVE_STATE(button)          (buttons[button].settings.activeState)
#define TIME_RELEASED_1(button)       (buttons[button].t1Released)
#define TIME_RELEASED_2(button)       (buttons[button].t2Released)
#define CLICK_TIME_DIFFERENCE(button) (TIME_RELEASED_1(button) - TIME_RELEASED_2(button))
#define DOUBLE_CLICK_TIME(button)     (buttons[button].settings.doubleClickMaxTimeDifference)
#define BUTTON_PRESSED                0x29
#define BUTTON_RELEASED               0x30

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */
void buttons_on_action_pressed(uint8_t button);
void buttons_on_action_released(uint8_t button);
uint8_t button_state(uint8_t button);

/* Public Functions */

void button_init(void) {

    // Add the debouncing tasks for each button to the button pressed list
    buttonPressedDebouncingTimerTasks[0] = &buttonUpPressedTmsTask;
    buttonPressedDebouncingTimerTasks[1] = &buttonDownPressedTmsTask;

    // Add the debouncing tasks for each button to the button released list
    buttonReleasedDebouncingTimerTasks[0] = &buttonUpReleasedTmsTask;
    buttonReleasedDebouncingTimerTasks[1] = &buttonDownReleasedTmsTask;

    // Enable the rising and falling interrupts on the EXTI interrupt lines
    // for the GPIO pins connected to each button. Rising and falling edges
    // on the button will now trigger an EXTI interrupt
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        button_enable(i);
    }
}

void button_enable(uint8_t button) {

    // Confirm a valid button index was given
    assert(button < NUM_BUTTONS);

    // Enable rising and falling interrupts on the given button
    EXTI->RTSR1 |= (0x01 << buttons[button].pin);
    EXTI->FTSR1 |= (0x01 << buttons[button].pin);
}

void button_disable(uint8_t button) {

    // Confirm a valid button index was given
    assert(button < NUM_BUTTONS);

    // Disable rising and falling interrupts on the given button
    EXTI->RTSR1 &= ~(0x01 << buttons[button].pin);
    EXTI->FTSR1 &= ~(0x01 << buttons[button].pin);
}

/* Private Functions */

void button_isr(uint8_t button) {

    // Confirm a valid button index was given
    assert(button < NUM_BUTTONS);

    // Check whether if the button is currently being pressed or released
    if (button_state(button) == BUTTON_PRESSED) {

        // Cancel any button released timer tasks that are still active. If
        // the button released timer task has not finished by now then the
        // button must be bouncing => we don't know the final state of the
        // button => cancel button being released and assume button has been
        // pressed.
        ts_cancel_recipe(buttonReleasedDebouncingTimerTasks[button]->id);

        // The button is automatically reset as soon as it has been released
        // for long enough time t1. If during a button press random oscillations
        // occur (due to bad hardware), the button will not have reset because
        // the oscillations are very short (< t1). Checking if the button is reset
        // prevents random oscillations from adding multiple a 'button pressed
        // Timer tasks' to the queue
        if (BUTTON_IS_RESET(button)) {

            // Add pressed timer task to timer.
            ts_add_recipe_to_queue(buttonPressedDebouncingTimerTasks[button]);
        }

        // Returning to ensure the button released code is not run
        return;
    }

    /* The button has been released if the code reaches this line */

    // Cancel any button pressed timer tasks that are still active. If
    // the button pressed timer task has not finished by now then the
    // button must be bouncing => we don't know the final state of the
    // button => cancel button being pressed and assume button has been
    // released.
    ts_cancel_recipe(buttonPressedDebouncingTimerTasks[button]->id);

    // Add released timer task to timer. If the button goes high before
    // the timer task completes then the task will be cancelled (meant
    // the buton was bouncing). If the button stays released for long
    // enough, the released timer task will finish and automatically call
    // the appropriate 'button released' function for the given button
    ts_add_recipe_to_queue(buttonReleasedDebouncingTimerTasks[button]);
}

void button_action_on_pressed(uint8_t button) {

    // Confirm a valid button index was given
    assert(button < NUM_BUTTONS);

    // Button has been pressed => button is no longer reset
    buttons[button].reset = FALSE;

    // If the previous button press happened < time t before
    // this button press occured then it is actually a 'double
    // click' not a 'single click' action. If it is a double
    // click then we want to cancel the 'single click' timer
    // task. If the previous button press happened > time t
    // before this button press then it was a single click
    // and that timer task will have already finished and left
    // the timer queue thus it doesn't matter if we try cancel
    // it
    ts_cancel_recipe(buttonSingleClickTimerTasks[button]->id);

    // Any button press may be a 'press and hold' button press.
    // Given this, we need to start a timer task for press and
    // hold. If the button remains pressed for the required
    // amount of time, the press and hold timer task will finish.
    // If the button is released before the press and hold timer
    // task can finish, it will be cancelled when the button is
    // released
    ts_add_recipe_to_queue(buttonPressAndHoldTimerTasks[button]);
}

void button_action_on_released(uint8_t button) {

    // Set the button to reset because it has been released
    buttons[button].reset = TRUE;

    // Record the current time that the given button was released.
    // If the difference between time 1 and time 2 < maxDoubleClickTime
    // for the given button then we know a double click has just occured
    TIME_RELEASED_2(button) = TIME_RELEASED_1(button);
    TIME_RELEASED_1(button) = HAL_GetTick();

    // Anytime the button is pressed, a press and hold timer task is added
    // to the queue. If we try cancel the press and hold timer task and
    // the function returns true then the timer task was still in the queue
    // and it was cancelled. If the queue still has that timer task in it
    // then we know the button was not pressed long enough to be a valid
    // 'press and hold' => either a double or a single click. If the function
    // returns 0 then the press and hold timer task must have finished and
    // been removed already which means it was a press and hold
    if (ts_cancel_recipe(buttonPressAndHoldTimerTasks[button]->id) == TRUE) {

        // If the difference between this button release and the last button
        // release < (max time difference between releases for a double click)
        // then this release must be the second release from a double click
        if (CLICK_TIME_DIFFERENCE(button) < DOUBLE_CLICK_TIME(button)) {

            // Call appropriate function for this button when a double click occurs
            (*buttonDoubleClickFunctions[button])();

            // Return so only the double click function is called
            return;
        }

        // If the press is not a double click, the press is either a single click
        // or it's the first press in a double click. Add single click timer task
        // to timer. If another click doesn't happen quickly enough the timer task
        // will call the appropriate function. If another click does happen quickly
        // enough, this task will be cancelled
        ts_add_recipe_to_queue(buttonSingleClickTimerTasks[button]);

        // Return so only the single click function is called
        return;
    }

    /* If the code reaches here, the button press type was 'press and hold' */

    // Call appropriate function for this button when a 'press and hold' has finished
}

uint8_t button_state(uint8_t button) {

    // Confirm a valid button index was given
    assert(button < NUM_BUTTONS);

    // Return the current state of the button
    if ((IDR(button) & (0x01 << PIN(button))) == ACTIVE_STATE(button)) {
        return BUTTON_PRESSED;
    }

    return BUTTON_RELEASED;
}