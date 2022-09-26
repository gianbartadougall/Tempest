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
#include "version_configuration.h"

// Import timer structures and public functions to be used by the buttons
#include "timer_ms.h"

// Import hardware configurations so the buttons GPIO port and pin for
// each button is known so the button states can be checked when required
#include "hardware_configuration.h"

/* Public STM Includes */
#include "stm32l4xx.h"

/* Public #defines */

/* Public Structures and Enumerations */

enum ButtonActiveState { ACTIVE_HIGH, ACTIVE_LOW };

typedef struct ButtonSettingsTypeDef {
    enum ButtonActiveState activeState;
    uint32_t minTimeBeforeReset;
    uint32_t minTimeBeforeActive;
} ButtonSettingsTypeDef;

typedef struct ButtonTypeDef {
    const GPIO_TypeDef* port;
    const uint32_t pin;
    const ButtonSettingsTypeDef settings;
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
    .activeState         = ACTIVE_HIGH,
    .minTimeBeforeReset  = 0,
    .minTimeBeforeActive = 0,
};

// Declare a structure for button 1
const struct ButtonTypeDef ButtonUp = {
    .port     = HC_BUTTON_0_PORT,
    .pin      = HC_BUTTON_0_PIN,
    .settings = ButtonSettings,
};

// Declare a structure for button 2
const struct ButtonTypeDef ButtonDown = {
    .port     = HC_BUTTON_1_PORT,
    .pin      = HC_BUTTON_1_PIN,
    .settings = ButtonSettings,
};

    /* Configure generic macros based on the number of buttons being used */
    #define NUM_BUTTONS 2

#endif

// Creating constant structures to be shared among tasks
const TimerTaskSettings PR_4_MUT_PAUSE_AWF_FINISH = {
    .priority               = TASK_MS_PRIORITY_4,
    .actionIfMoreUrgentTask = TIMER_MS_TASK_PAUSE,
    .actionWhenFinished     = TIMER_MS_TASK_FINISH,
};

const TimerTaskStatus INITIAL_STATUS_VALUES = {
    .index          = 0,
    .status         = 0,
    .delayRemaining = 0,
};

/** Defining the functions to be called when button 1 is pressed and
 *  released
 */
struct TimerMsTask1 buttonUpPressedTmsTask = {
    .id       = 0,
    .tasks    = {},
    .delays   = {},
    .size     = 0,
    .settings = PR_4_MUT_PAUSE_AWF_FINISH,
    .status   = INITIAL_STATUS_VALUES,
};

struct TimerMsTask1 buttonUpReleasedTmsTask = {
    .id       = 1,
    .tasks    = {},
    .delays   = {},
    .size     = 0,
    .settings = PR_4_MUT_PAUSE_AWF_FINISH,
    .status   = INITIAL_STATUS_VALUES,
};

struct TimerMsTask1 buttonDownPressedTmsTask = {
    .id       = 0,
    .tasks    = {},
    .delays   = {},
    .size     = 0,
    .settings = PR_4_MUT_PAUSE_AWF_FINISH,
    .status   = INITIAL_STATUS_VALUES,
};

struct TimerMsTask1 buttonDownReleasedTmsTask = {
    .id       = 1,
    .tasks    = {},
    .delays   = {},
    .size     = 0,
    .settings = PR_4_MUT_PAUSE_AWF_FINISH,
    .status   = INITIAL_STATUS_VALUES,
};

TimerMsTask1* buttonPressedTimerTasks[NUM_BUTTONS];
TimerMsTask1* buttonReleasedTimerTasks[NUM_BUTTONS];
ButtonTypeDef buttons[NUM_BUTTONS] = {ButtonUp, ButtonDown};

/* Preprocessor statments to check certain configuration settings to detect any mistakes */
#ifndef NUM_BUTTONS
    #error "Number of buttons has not been defined"
#endif

#endif // BUTTON_CONFIGURATION_H
