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

/* Private Includes */
#include "button_configuration.h"

/* Private #defines */

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */

/* Public Functions */

void button_init(void) {

    // Initialise the list holding all the timer tasks for the buttons
    // when they are pressed
    buttonPressedTimerTasks[0] = &buttonUpPressedTmsTask;
    buttonPressedTimerTasks[1] = &buttonDownPressedTmsTask;

    // Initialise the list holding all the timer tasks for the buttons
    // when they release
    buttonReleasedTimerTasks[0] = &buttonUpReleasedTmsTask;
    buttonReleasedTimerTasks[1] = &buttonDownReleasedTmsTask;

    // Initialise the ISRs of the GPIO pins that are used by both buttons
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    }
}

/* Private Functions */
