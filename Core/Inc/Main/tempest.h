/**
 * @file tempest.h
 * @author Gian Barta-Dougall
 * @brief System file for tempest
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef TEMPEST_H
#define TEMPEST_H

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
void tempest_init(void);

void tempest_update(void);
void tempest_stop_blind_1_moving(void);
void tempest_stop_blind_2_moving(void);

#endif // TEMPEST_H
