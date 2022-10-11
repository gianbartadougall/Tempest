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
#include "task_scheduler_1_id_config.h"

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
    uint8_t reset;
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
    .doubleClickMaxTimeDifference = 500,
};

// Declare a structure for button 1
const struct ButtonTypeDef ButtonUp = {
    .port       = HC_BUTTON_1_PORT,
    .pin        = HC_BUTTON_1_PIN,
    .reset      = TRUE,
    .t1Released = 0,
    .t2Released = 0,
    .settings   = ButtonSettings,
};

// Declare a structure for button 2
const struct ButtonTypeDef ButtonDown = {
    .port       = HC_BUTTON_2_PORT,
    .pin        = HC_BUTTON_2_PIN,
    .reset      = TRUE,
    .t1Released = 0,
    .t2Released = 0,
    .settings   = ButtonSettings,
};

    /* Configure generic macros based on the number of buttons being used */
    #define NUM_BUTTONS 1

#endif

/******************** Button 1 Pressed Process ********************/
const struct Task1 bUpPressed = {
    .processId  = TS_ID_BUTTON_UP_PRESSED,
    .delay      = 10,
    .functionId = 0,
    .group      = BUTTON_GROUP,
    .nextTask   = NULL,
};
/******************************************************************/

/******************** Button 1 Released Process ********************/
const struct Task1 bUpReleased = {
    .processId  = TS_ID_BUTTON_UP_RELEASED,
    .delay      = 10,
    .functionId = 1,
    .group      = BUTTON_GROUP,
    .nextTask   = NULL,
};
/*******************************************************************/

/******************** Button 1 Single Click Process ********************/
const struct Task1 bUpSingleClick = {
    .processId  = TS_ID_BUTTON_UP_SINGLE_CLICK,
    .delay      = 150,
    .functionId = 2,
    .group      = BUTTON_GROUP,
    .nextTask   = NULL,
};
/***********************************************************************/

/******************** Button 1 Press and Hold Process ********************/
const struct Task1 bUpPressAndHold = {
    .processId  = TS_ID_BUTTON_UP_PRESS_AND_HOLD,
    .delay      = 2000,
    .functionId = 3,
    .group      = BUTTON_GROUP,
    .nextTask   = NULL,
};
/*************************************************************************/

void nullFunction(void) {
    // debug_prints("Double click\r\n");
}

void (*bDoubleClickFunctions[NUM_BUTTONS])(void) = {&nullFunction};

Task1 bPressedDebounceTasks[NUM_BUTTONS]  = {bUpPressed};
Task1 bReleasedDebounceTasks[NUM_BUTTONS] = {bUpReleased};
Task1 bSingleClickTasks[NUM_BUTTONS]      = {bUpSingleClick};
Task1 bPressAndHoldTasks[NUM_BUTTONS]     = {bUpPressAndHold};

ButtonTypeDef buttons[NUM_BUTTONS] = {ButtonUp};

/* Preprocessor statments to check certain configuration settings to detect any mistakes */
#ifndef NUM_BUTTONS
    #error "Number of buttons has not been defined"
#endif

#endif // BUTTON_CONFIGURATION_H
