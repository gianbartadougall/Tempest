/**
 * @file synchronous_interrupts.h
 * @author Gian Barta-Dougall
 * @brief System file for synchronous_interrupts.c
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef SYNCHRONOUS_INTERRUPTS_H
#define SYNCHRONOUS_INTERRUPTS_H

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
void sychronous_interrupts_init(void);

void sync_timer_delay10us(uint16_t delay10us);

#endif // SYNCHRONOUS_INTERRUPTS_H
