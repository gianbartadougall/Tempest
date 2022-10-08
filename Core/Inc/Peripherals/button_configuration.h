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
#include "task_scheduler.h"

// Import hardware configurations so the buttons GPIO port and pin for
// each button is known so the button states can be checked when required
#include "hardware_config.h"

// Imports for #defines such as TRUE and FALSE
#include "utilities.h"

/* Public STM Includes */
#include "stm32l4xx.h"

/* Public #defines */

/* Public Structures and Enumerations */

enum ButtonActiveState { ACTIVE_HIGH, ACTIVE_LOW };

typedef struct ButtonSettingsTypeDef {
    enum ButtonActiveState activeState;
    uint32_t minTimeBeforeReleased;
    uint32_t minTimeBeforePressed;
    uint16_t doubleClickMaxTimeDifference;
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
    .activeState                  = ACTIVE_HIGH,
    .minTimeBeforeReleased        = 0,
    .minTimeBeforePressed         = 0,
    .doubleClickMaxTimeDifference = 10,
};

// Declare a structure for button 1
const struct ButtonTypeDef ButtonUp = {
    .port       = HC_BUTTON_0_PORT,
    .pin        = HC_BUTTON_0_PIN,
    .reset      = TRUE,
    .t1Released = 0,
    .t2Released = 0,
    .settings   = ButtonSettings,
};

// Declare a structure for button 2
const struct ButtonTypeDef ButtonDown = {
    .port       = HC_BUTTON_1_PORT,
    .pin        = HC_BUTTON_1_PIN,
    .reset      = TRUE,
    .t1Released = 0,
    .t2Released = 0,
    .settings   = ButtonSettings,
};

    /* Configure generic macros based on the number of buttons being used */
    #define NUM_BUTTONS 2

#endif

/** Defining the functions to be called when button 1 is pressed and
 *  released
 */
struct Recipe buttonUpPressedTmsTask = {
    .id          = 0,
    .functionIds = {},
    .delays      = {},
    .numTasks    = 0,
};

struct Recipe buttonUpReleasedTmsTask = {
    .id          = 1,
    .functionIds = {},
    .delays      = {},
    .numTasks    = 0,
};

struct Recipe buttonUpSingleClickTmsTask = {
    .id          = 2,
    .functionIds = {},
    .delays      = {},
    .numTasks    = 0,
};

struct Recipe buttonUpDoubleClickTmsTask = {
    .id          = 3,
    .functionIds = {},
    .delays      = {},
    .numTasks    = 0,
};

struct Recipe buttonUpPressAndHoldTmsTask = {
    .id          = 4,
    .functionIds = {},
    .delays      = {},
    .numTasks    = 0,
};

struct Recipe buttonDownPressedTmsTask = {
    .id          = 5,
    .functionIds = {},
    .delays      = {},
    .numTasks    = 0,
};

struct Recipe buttonDownReleasedTmsTask = {
    .id          = 6,
    .functionIds = {},
    .delays      = {},
    .numTasks    = 0,
};

struct Recipe buttonDownSingleClickTmsTask = {
    .id          = 7,
    .functionIds = {},
    .delays      = {},
    .numTasks    = 0,
};

struct Recipe buttonDownDoubleClickTmsTask = {
    .id          = 8,
    .functionIds = {},
    .delays      = {},
    .numTasks    = 0,
};

struct Recipe buttonDownPressAndHoldTmsTask = {
    .id          = 9,
    .functionIds = {},
    .delays      = {},
    .numTasks    = 0,
};

void nullFunction(void) {}

void (*buttonDoubleClickFunctions[NUM_BUTTONS])(void) = {&nullFunction, &nullFunction};

Recipe* buttonPressedDebouncingTimerTasks[NUM_BUTTONS];
Recipe* buttonReleasedDebouncingTimerTasks[NUM_BUTTONS];
Recipe* buttonSingleClickTimerTasks[NUM_BUTTONS];
Recipe* buttonPressAndHoldTimerTasks[NUM_BUTTONS];

ButtonTypeDef buttons[NUM_BUTTONS] = {ButtonUp, ButtonDown};

/* Preprocessor statments to check certain configuration settings to detect any mistakes */
#ifndef NUM_BUTTONS
    #error "Number of buttons has not been defined"
#endif

#endif // BUTTON_CONFIGURATION_H
