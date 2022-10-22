/**
 * @file button_configuration.h
 * @author Gian Barta-Dougall
 * @brief System file for button_configuration
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef BUTTON_CONFIGURATION_H
#define BUTTON_CONFIGURATION_H

/* Public Includes */

/**
 * Including all the header files that contain the functions that
 * will be called by the buttons when they are pressed/released
 */

// Including timer file to get access to the timer structure and

// Import the version to compile so the correct hardware is used for the buttons
#include "version_config.h"

// Import timer structures and public functions to be used by the buttons
#include "task_scheduler_1.h"

// Import hardware configurations so the buttons GPIO port and pin for
// each button is known so the button states can be checked when required
#include "hardware_config.h"

// Imports for #defines such as TRUE and FALSE
#include "utilities.h"

/* Public STM Includes */
#include "stm32l4xx.h"

#include "button.h"

/* Public Macros */
#define BUTTON_ACTIVE_LOW  0
#define BUTTON_ACTIVE_HIGH 1

/* Public Structures and Enumerations */

typedef struct ButtonSettingsTypeDef {
    const uint8_t activeState;
    const uint16_t doubleClickMaxTimeDifference;
    uint32_t minTimeBeforeReleased;
    uint32_t minTimeBeforePressed;
} ButtonSettingsTypeDef;

typedef struct ButtonTypeDef {
    const GPIO_TypeDef* port;
    const uint32_t pin;
    const ButtonSettingsTypeDef settings;
    uint32_t t1Released;
    uint32_t t2Released;
} ButtonTypeDef;

/**
 * Hardware version 0 for Tempest has 2 buttons onboard.
 * The following section creates the button structures that
 * contain their port information (used so this driver can
 * check the state of the buttons when required) and their
 * active state. The active state high => button will pull
 * GPIO pin high when pressed. Active state low => button
 * will pull GPIO pin low when pressed.
 */
#if (VERSION_MAJOR == 0)

const struct ButtonSettingsTypeDef ButtonSettings = {
    .activeState                  = BUTTON_ACTIVE_HIGH,
    .minTimeBeforeReleased        = 0,
    .minTimeBeforePressed         = 0,
    .doubleClickMaxTimeDifference = 300,
};

// Declare a structure for button 1
const struct ButtonTypeDef ButtonUp = {
    .port       = HC_BUTTON_1_PORT,
    .pin        = HC_BUTTON_1_PIN,
    .t1Released = 0,
    .t2Released = 0,
    .settings   = ButtonSettings,
};

// Declare a structure for button 2
const struct ButtonTypeDef ButtonDown = {
    .port       = HC_BUTTON_2_PORT,
    .pin        = HC_BUTTON_2_PIN,
    .t1Released = 0,
    .t2Released = 0,
    .settings   = ButtonSettings,
};

    /* Configure generic macros based on the number of buttons being used */
    #define NUM_BUTTONS 2

#endif

/******************** Button 1 Process ISR ********************/
/**
 * @note Delay for short period of time before processing ISR. Processing ISR means
 * determining whether the interrupt was a rising or falling edge and calling
 * appropriate functions based on the rising/falling interrupt. Experimentally,
 * 12ms delay worked quite well as it gave  enough time for the button to stop
 * debouncing but not too long such that it would prevent ISR from occuring if
 * the button was repeatedly click very quicky
 */
const struct Task1 bUpProcessISR = {
    .delay      = 12, // Experimentally 12 seemed to be a good delay time
    .functionId = FUNC_ID_BUTTON_UP_PROCESS_ISR,
    .group      = BUTTON_GROUP,
    .nextTask   = NULL,
};
/******************************************************************/

/******************** Button 1 Single Click Process ********************/
/**
 * @note Sets the flag signalling a single click occured after waiting a
 * delay. The delay is one more than the maximum time difference between
 * clicks for a double click to be deemed valid. If the delay was <=
 * max time difference for a double click then a double click would
 * trigger a single click as well as a double click
 */
const struct Task1 bUpSingleClick = {
    .delay      = (ButtonUp.settings.doubleClickMaxTimeDifference + 1),
    .functionId = FUNC_ID_BUTTON_UP_SINGLE_CLICK,
    .group      = BUTTON_GROUP,
    .nextTask   = NULL,
};
/***********************************************************************/

/******************** Button 1 Press and Hold Process ********************/
/**
 * @note Sets the flag signalling a press and hold has occured after
 * waiting the given delay. The delay must be larger than the double
 * and single click delays. The larger this delay is in comparison
 * to the single click delay, the longer you can hold the button and
 * then release before it will register as a press and hold
 */
const struct Task1 bUpPressAndHold = {
    .delay      = 2000,
    .functionId = FUNC_ID_BUTTON_UP_PRESS_AND_HOLD,
    .group      = BUTTON_GROUP,
    .nextTask   = NULL,
};
/*************************************************************************/

/******************** Button 2 Process ISR ********************/
/**
 * @note Refer to Button 1 Process ISR task for a description
 */
const struct Task1 bDownProcessISR = {
    .delay      = 12,
    .functionId = FUNC_ID_BUTTON_DOWN_PROCESS_ISR,
    .group      = BUTTON_GROUP,
    .nextTask   = NULL,
};
/******************************************************************/

/******************** Button 2 Single Click Process ********************/
/**
 * @note Refer to Button 2 Single Click Process task for a description
 */
const struct Task1 bDownSingleClick = {
    .delay      = ButtonDown.settings.doubleClickMaxTimeDifference + 1,
    .functionId = FUNC_ID_BUTTON_DOWN_SINGLE_CLICK,
    .group      = BUTTON_GROUP,
    .nextTask   = NULL,
};
/***********************************************************************/

/******************** Button 2 Press and Hold Process ********************/
/**
 * @note Refer to Button 2 Press and Hold Process task for a description
 */
const struct Task1 bDownPressAndHold = {
    .delay      = 2000,
    .functionId = FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD,
    .group      = BUTTON_GROUP,
    .nextTask   = NULL,
};
/*************************************************************************/

Task1 bSingleClickTasks[NUM_BUTTONS]  = {bUpSingleClick, bDownSingleClick};
Task1 bPressAndHoldTasks[NUM_BUTTONS] = {bUpPressAndHold, bDownPressAndHold};
Task1 bProcessISRTasks[NUM_BUTTONS]   = {bUpProcessISR, bDownProcessISR};

ButtonTypeDef buttons[NUM_BUTTONS] = {ButtonUp, ButtonDown};

#define BUTTON_UP_INDEX   0
#define BUTTON_DOWN_INDEX 1

/* Preprocessor statments to check certain configuration settings to detect any mistakes */
#ifndef NUM_BUTTONS
    #error "Number of buttons has not been defined"
#endif

#endif // BUTTON_CONFIGURATION_H
