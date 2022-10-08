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

/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void button_init(void);

void button_enable(uint8_t button);

#endif // BUTTON_H
