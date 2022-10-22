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
#define PORT(index)                      (buttons[index].port)
#define IDR(index)                       (PORT(index)->IDR)
#define PIN(index)                       (buttons[index].pin)
#define BUTTON_IS_RESET(index)           (buttons[index].reset == 1)
#define ACTIVE_STATE(index)              (buttons[index].settings.activeState)
#define TIME_RELEASED_1(index)           (buttons[index].t1Released)
#define TIME_RELEASED_2(index)           (buttons[index].t2Released)
#define CLICK_TIME_DIFFERENCE(index)     (TIME_RELEASED_1(index) - TIME_RELEASED_2(index))
#define MAX_TIME_FOR_DOUBLE_CLICK(index) (buttons[index].settings.doubleClickMaxTimeDifference)

// #define BUTTON_ID_INVALID(id) (((id < BUTTON_ID_OFFSET) || (id > (NUM_BUTTONS - 1 + BUTTON_ID_OFFSET))) ? 1 : 0)

/* Private Structures and Enumerations */

/* Private Variable Declarations */
extern uint32_t buttonTasksFlag;

uint32_t timeReleased1[NUM_BUTTONS] = {0};
uint32_t timeReleased2[NUM_BUTTONS] = {0};
uint8_t edge                        = 0;

/* Private Function Prototypes */
void button_on_action_pressed(uint8_t button);
void button_on_action_released(uint8_t button);
void button_enable_interrupt(uint8_t buttonId);
void button_disable_interrupt(uint8_t buttonId);

/* Public Functions */

void button_init(void) {

    // Enable button interrupts
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        button_enable_interrupt(i);
    }
}

void button_enable_interrupt(uint8_t bIndex) {
    // Enable interrupts by unmasking them
    EXTI->PR1 = (0x01 << buttons[bIndex].pin); // Clear any pending interrupts
    EXTI->IMR1 |= (0x01 << buttons[bIndex].pin);
}

void button_disable_interrupt(uint8_t bIndex) {
    // Disable interrupts by masking them
    EXTI->IMR1 &= ~(0x01 << buttons[bIndex].pin);
}

uint8_t button_get_state(uint8_t index) {

    // Return the current state of the button
    if (PIN_IDR_STATE(buttons[index].port, buttons[index].pin) == ACTIVE_STATE(index)) {
        return BUTTON_PRESSED;
    }

    return BUTTON_RELEASED;
}

/* Private Functions */

void button_isr(uint8_t button) {

    // Disable interrupt for given button. This ensures that bouncing does not
    // call multiple interrupts
    button_disable_interrupt(button);

    // Schedule processing of ISR to occur after a small delay. All button bouncing
    // should finish before the delay finishes => when the ISr is processed and the
    // IDR is read, the reading is reliable. From testing, reading the IDR in the ISR
    // is not 100% reliable because bounces can be very quick (~10us) - quick
    // enough for signal to bounce back to the wrong value when you read the ISR
    // giving a wrong reading
    ts_add_task_to_queue(&bProcessISRTasks[button]);

    return;
}

void button_action_on_pressed(uint8_t index) {
    // debug_prints(("  !!! PRESSED !!!\r\n\r\n");

    // If the previous button press happened < time t before
    // this button press occured then it is actually a 'double
    // click' not a 'single click' action. If it is a double
    // click then we want to cancel the 'single click' timer
    // task. If the previous button press happened > time t
    // before this button press then it was a single click
    // and that timer task will have already finished and left
    // the timer queue thus it doesn't matter if we try cancel
    // it
    ts_cancel_running_task(&bSingleClickTasks[index]);

    // Any button press may be a 'press and hold' button press.
    // Given this, we need to start a timer task for press and
    // hold. If the button remains pressed for the required
    // amount of time, the press and hold timer task will finish.
    // If the button is released before the press and hold timer
    // task can finish, it will be cancelled when the button is
    // released
    // // debug_prints(("Start press and hold\r\n");
    ts_add_task_to_queue(&bPressAndHoldTasks[index]);
}

void button_on_action_released(uint8_t index) {

    // Record the current time that the given button was released.
    // If the difference between time 1 and time 2 < maxDoubleClickTime
    // for the given button then we know a double click has just occured
    TIME_RELEASED_2(index) = TIME_RELEASED_1(index);
    TIME_RELEASED_1(index) = HAL_GetTick();

    // Anytime the button is pressed, a press and hold timer task is added
    // to the queue. If we try cancel the press and hold timer task and
    // the function returns true then the timer task was still in the queue
    // and it was cancelled. If the queue still has that timer task in it
    // then we know the button was not pressed long enough to be a valid
    // 'press and hold' => either a double or a single click. If the function
    // returns 0 then the press and hold timer task must have finished and
    // been removed already which means it was a press and hold
    if (ts_cancel_running_task(&bPressAndHoldTasks[index]) == TS_TASK_CANCELLED) {

        // If the difference between this button release and the last button
        // release < (max time difference between releases for a double click)
        // then this release must be the second release from a double click
        if (CLICK_TIME_DIFFERENCE(index) <= MAX_TIME_FOR_DOUBLE_CLICK(index)) {

            // Call appropriate function for this button when a double click occurs
            if (index == BUTTON_UP_INDEX) {
                FLAG_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_DOUBLE_CLICK);
            } else if (index == BUTTON_DOWN_INDEX) {
                FLAG_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_DOUBLE_CLICK);
            }
            // debug_prints("Double Click\r\n");
            // char m[60];
            // sprintf(m, "B%i - Double Click\r\n", index);
            // debug_prints(m);
            // (*bDoubleClickFunctions[bi])();
            // Return so only the double click function is called
            return;
        }

        // If the press is not a double click, the press is either a single click
        // or it's the first press in a double click. Add single click timer task
        // to timer. If another click doesn't happen quickly enough the timer task
        // will call the appropriate function. If another click does happen quickly
        // enough, this task will be cancelled
        ts_add_task_to_queue(&bSingleClickTasks[index]);

        // debug_prints(("Maybe single click\r\n");
        // Return so only the single click function is called
        return;
    }

    // debug_prints(("Press and hold\r\n");

    /* If the code reaches here, the button press type was 'press and hold' */
    if (index == BUTTON_UP_INDEX) {
        FLAG_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD_RELEASED);
    } else if (index == BUTTON_DOWN_INDEX) {
        FLAG_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD_RELEASED);
    }

    // Call appropriate function for this button when a 'press and hold' has finished
}

void button_process_internal_flags(void) {

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PROCESS_ISR)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_PROCESS_ISR);

        button_enable_interrupt(BUTTON_UP_INDEX);

        // Determine whether the ISR was a rising or falling edge
        if (button_get_state(BUTTON_UP_INDEX) == BUTTON_PRESSED) {
            button_action_on_pressed(BUTTON_UP_INDEX);
        } else {
            button_on_action_released(BUTTON_UP_INDEX);
        }
    }

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PROCESS_ISR)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PROCESS_ISR);

        button_enable_interrupt(BUTTON_DOWN_INDEX);

        // Determine whether the ISR was a rising or falling edge
        if (button_get_state(BUTTON_DOWN_INDEX) == BUTTON_PRESSED) {
            button_action_on_pressed(BUTTON_DOWN_INDEX);
        } else {
            button_on_action_released(BUTTON_DOWN_INDEX);
        }
    }
}