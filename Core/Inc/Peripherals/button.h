/**
 * @file button.h
 * @author Gian Barta-Dougall
 * @brief System file for button
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef BUTTON_H
#define BUTTON_H

/* Public Includes */

/* Public STM Includes */
#include "stm32l4xx.h"

/* Public #defines */
enum ButtonFunctions {
    FUNC_ID_BUTTON_UP_PROCESS_ISR,
    FUNC_ID_BUTTON_UP_SINGLE_CLICK,
    FUNC_ID_BUTTON_UP_DOUBLE_CLICK,
    FUNC_ID_BUTTON_UP_PRESS_AND_HOLD,
    FUNC_ID_BUTTON_UP_PRESS_AND_HOLD_RELEASED,
    FUNC_ID_BUTTON_DOWN_PROCESS_ISR,
    FUNC_ID_BUTTON_DOWN_SINGLE_CLICK,
    FUNC_ID_BUTTON_DOWN_DOUBLE_CLICK,
    FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD,
    FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD_RELEASED,
};

#define BUTTON_PRESSED  0x29
#define BUTTON_RELEASED 0x30

// Ids used by external files to access information on the buttons
#define BUTTON_UP_ID   0
#define BUTTON_DOWN_ID 1

/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void button_init(void);

void button_isr(uint8_t buttonId);

/**
 * @brief Processes flags that activate internal processes
 * in the button .c file. This function needs to run often
 * to ensure automatic processes like button debouncing are
 * handled correctly
 */
void button_process_internal_flags(void);

uint8_t button_get_state(uint8_t buttonId);

#endif // BUTTON_H
