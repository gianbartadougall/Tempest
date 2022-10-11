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

#define BUTTON_ID_OFFSET 74
#define BUTTON_UP        (0 + BUTTON_ID_OFFSET)
#define BUTTON_DOWN      (1 + BUTTON_ID_OFFSET)

/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void button_init(void);

void button_isr(uint8_t buttonId);

void button_process_flags();

#endif // BUTTON_H
