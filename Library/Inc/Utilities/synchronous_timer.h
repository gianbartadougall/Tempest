/**
 * @file synchronous_timer.h
 * @author Gian Barta-Dougall
 * @brief System file for synchronous_timer
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef SYNCHRONOUS_TIMER_H
#define SYNCHRONOUS_TIMER_H

/* Public Includes */

/* Public STM Includes */
#include "stm32l4xx.h"

/* Public #defines */

/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Enable the timer which lets the interrupts run
 */
void synchronous_timer_enable(void);

/**
 * @brief Disable the timer which stops the interrupts from running
 */
void synchronous_timer_disable(void);

#endif // SYNCHRONOUS_TIMER_H
