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
#include "utilities.h"

/* Private Includes */
#include "button_configuration.h"

/* Private macros */
#define PORT(index)                  (buttons[index].port)
#define IDR(index)                   (PORT(index)->IDR)
#define PIN(index)                   (buttons[index].pin)
#define BUTTON_IS_RESET(index)       (buttons[index].reset == 1)
#define ACTIVE_STATE(index)          (buttons[index].settings.activeState)
#define TIME_RELEASED_1(index)       (buttons[index].t1Released)
#define TIME_RELEASED_2(index)       (buttons[index].t2Released)
#define CLICK_TIME_DIFFERENCE(index) (TIME_RELEASED_1(index) - TIME_RELEASED_2(index))
#define DOUBLE_CLICK_TIME(index)     (buttons[index].settings.doubleClickMaxTimeDifference)
#define BUTTON_PRESSED               0x29
#define BUTTON_RELEASED              0x30

#define BUTTON_ID_INVALID(id) (((id < BUTTON_ID_OFFSET) || (id > (NUM_BUTTONS - 1 + BUTTON_ID_OFFSET))) ? 1 : 0)

/* Private Structures and Enumerations */

/* Private Variable Declarations */
extern uint32_t buttonTasksFlag;

/* Private Function Prototypes */
void button_on_action_pressed(uint8_t button);
void button_on_action_released(uint8_t button);
uint8_t button_state(uint8_t button);
void button_enable_interrupt(uint8_t buttonId);
void button_disable_interrupt(uint8_t buttonId);

/* Public Functions */

void button_init(void) {

    // Enable button interrupts
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        button_enable_interrupt(i);
    }
}

void button_enable_interrupt(uint8_t button) {

    // Confirm a valid button index was given
    if (button >= NUM_BUTTONS) {
        return;
    }

    // Enable interrupts by unmasking them
    EXTI->IMR1 |= (0x01 << buttons[button].pin);
}

void button_disable_interrupt(uint8_t button) {

    // Confirm a valid button index was given
    if (button >= NUM_BUTTONS) {
        return;
    }

    // Disable interrupts by masking them
    EXTI->IMR1 &= ~(0x01 << buttons[button].pin);
}

/* Private Functions */

void button_isr(uint8_t button) {

    // Confirm a valid button index was given
    if (button >= NUM_BUTTONS) {
        return;
    }

    // Check whether if the button is currently being pressed or released
    if (button_state(button) == BUTTON_PRESSED) {
        // uint32_t time1 = TIM15->CNT;

        if (ts_task_is_running(&bPressedDebounceTasks[button])) {
            // debug_prints("G  ");
            return;
        }
        // debug_prints("1  ");
        // Cancel any button released timer tasks that are still active. If
        // the button released timer task has not finished by now then the
        // button must be bouncing => we don't know the final state of the
        // button => cancel button being released and assume button has been
        // pressed.
        if (ts_cancel_running_task(&bReleasedDebounceTasks[button])) {
            // debug_prints("2  ");
        }

        // The button is automatically reset as soon as it has been released
        // for long enough time t1. If during a button press random oscillations
        // occur (due to bad hardware), the button will not have reset because
        // the oscillations are very short (< t1). Checking if the button is reset
        // prevents random oscillations from adding multiple a 'button pressed
        // Timer tasks' to the queue
        if (BUTTON_IS_RESET(button)) {

            // Add pressed timer task to timer.
            // debug_prints("3  ");
            ts_add_task_to_queue(&bPressedDebounceTasks[button]);
        } else {
            // debug_prints("9  ");
        }
        // uint32_t time2 = TIM15->CNT;
        // char m[60];
        // sprintf(m, "T@ %li\tTime to add to queue: %li\r\n", time2, time2 - time1);
        // debug_prints(m);

        // Returning to ensure the button released code is not run
        return;
    }

    // Check if there is a button released task already running
    if (ts_task_is_running(&bReleasedDebounceTasks[button])) {
        // debug_prints("F  ");
        return;
    }

    // debug_prints("4  ");
    /* The button has been released if the code reaches this line */

    // Cancel any button pressed timer tasks that are still active. If
    // the button pressed timer task has not finished by now then the
    // button must be bouncing => we don't know the final state of the
    // button => cancel button being pressed and assume button has been
    // released.
    if (ts_cancel_running_task(&bPressedDebounceTasks[button])) {
        // debug_prints("5  ");
    }

    // Add released timer task to timer. If the button goes high before
    // the timer task completes then the task will be cancelled (meant
    // the buton was bouncing). If the button stays released for long
    // enough, the released timer task will finish and automatically call
    // the appropriate 'button released' function for the given button
    // debug_prints("6  ");
    ts_add_task_to_queue(&bReleasedDebounceTasks[button]);
}

void button_action_on_pressed(uint8_t buttonId) {
    debug_prints("  !!! PRESSED !!!\r\n\r\n");
    // Confirm a valid button index was given
    if (BUTTON_ID_INVALID(buttonId)) {
        return;
    }

    uint8_t bi = buttonId - BUTTON_ID_OFFSET;
    // Button has been pressed => button is no longer reset
    buttons[bi].reset = FALSE;

    // If the previous button press happened < time t before
    // this button press occured then it is actually a 'double
    // click' not a 'single click' action. If it is a double
    // click then we want to cancel the 'single click' timer
    // task. If the previous button press happened > time t
    // before this button press then it was a single click
    // and that timer task will have already finished and left
    // the timer queue thus it doesn't matter if we try cancel
    // it
    ts_cancel_running_task(&bSingleClickTasks[bi]);

    // Any button press may be a 'press and hold' button press.
    // Given this, we need to start a timer task for press and
    // hold. If the button remains pressed for the required
    // amount of time, the press and hold timer task will finish.
    // If the button is released before the press and hold timer
    // task can finish, it will be cancelled when the button is
    // released
    // debug_prints("Start press and hold\r\n");
    ts_add_task_to_queue(&bPressAndHoldTasks[bi]);
}

void button_on_action_released(uint8_t buttonId) {

    if (BUTTON_ID_INVALID(buttonId)) {
        return;
    }

    uint8_t bi = buttonId - BUTTON_ID_OFFSET;
    // Set the button to reset because it has been released
    buttons[bi].reset = TRUE;

    // Record the current time that the given button was released.
    // If the difference between time 1 and time 2 < maxDoubleClickTime
    // for the given button then we know a double click has just occured
    TIME_RELEASED_2(bi) = TIME_RELEASED_1(bi);
    TIME_RELEASED_1(bi) = HAL_GetTick();

    // Anytime the button is pressed, a press and hold timer task is added
    // to the queue. If we try cancel the press and hold timer task and
    // the function returns true then the timer task was still in the queue
    // and it was cancelled. If the queue still has that timer task in it
    // then we know the button was not pressed long enough to be a valid
    // 'press and hold' => either a double or a single click. If the function
    // returns 0 then the press and hold timer task must have finished and
    // been removed already which means it was a press and hold
    if (ts_cancel_running_task(&bPressAndHoldTasks[bi]) == TRUE) {

        // If the difference between this button release and the last button
        // release < (max time difference between releases for a double click)
        // then this release must be the second release from a double click
        if (CLICK_TIME_DIFFERENCE(bi) < DOUBLE_CLICK_TIME(bi)) {

            // Call appropriate function for this button when a double click occurs
            (*bDoubleClickFunctions[bi])();
            debug_prints("Double click\r\n");
            // Return so only the double click function is called
            return;
        }

        // If the press is not a double click, the press is either a single click
        // or it's the first press in a double click. Add single click timer task
        // to timer. If another click doesn't happen quickly enough the timer task
        // will call the appropriate function. If another click does happen quickly
        // enough, this task will be cancelled
        ts_add_task_to_queue(&bSingleClickTasks[bi]);
        debug_prints("Maybe single click\r\n");
        // Return so only the single click function is called
        return;
    }

    debug_prints("Press and hold\r\n");

    /* If the code reaches here, the button press type was 'press and hold' */

    // Call appropriate function for this button when a 'press and hold' has finished
}

uint8_t button_state(uint8_t button) {

    // Confirm a valid button index was given
    if (button >= NUM_BUTTONS) {
        return 255;
    }

    // Return the current state of the button
    uint8_t state = ((IDR(button) & (0x01 << PIN(button))) == 0) ? 0 : 1;
    if (state == ACTIVE_STATE(button)) {
        return BUTTON_PRESSED;
    }

    return BUTTON_RELEASED;
}

void button_process_flags(void) {
    if (buttonTasksFlag & (0x01 << 0)) {
        button_action_on_pressed(BUTTON_UP);
        buttonTasksFlag &= ~(0x01 << 0);
    }

    if (buttonTasksFlag & (0x01 << 1)) {
        button_on_action_released(BUTTON_UP);
        buttonTasksFlag &= ~(0x01 << 1);
    }

    if (buttonTasksFlag & (0x01 << 2)) {
        debug_prints("Single click\r\n");
        buttonTasksFlag &= ~(0x01 << 2);
    }

    if (buttonTasksFlag & (0x01 << 3)) {
        debug_prints("Press and hold!\r\n");
        buttonTasksFlag &= ~(0x01 << 3);
    }
}