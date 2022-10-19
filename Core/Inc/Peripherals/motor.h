/**
 * @file motor.h
 * @author Gian Barta-Dougall
 * @brief System file for motor
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef MOTOR_H
#define MOTOR_H

/* Public Includes */

/* Public STM Includes */
#include "stm32l4xx.h"

/* Public #defines */

// Add an offset to each ID supplied. This makes it harder to accidently parse an incorrect
// ID that
#define MOTOR_ID_OFFSET 113
#define MOTOR_1_ID      (0 + MOTOR_ID_OFFSET)
#define MOTOR_2_ID      (1 + MOTOR_ID_OFFSET)

/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void motor_init(void);

void motor_forward(uint8_t motorId);
void motor_reverse(uint8_t motorId);
void motor_brake(uint8_t motorId);
void motor_stop(uint8_t motorId);

#endif // MOTOR_H
