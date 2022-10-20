/**
 * @file blind_motor.h
 * @author Gian Barta-Dougall
 * @brief This file is a wrapper file which handles the logic for the rotary
 * encoder coupled with the motor. The purpose of this file is to abstract
 * away the intracacies of operating the motor and encoder together
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef BLIND_MOTOR_H
#define BLIND_MOTOR_H

/* Public Includes */

/* Public STM Includes */
#include "stm32l4xx.h"

/* Public #defines */
typedef struct BlindMotor {
    uint8_t blindId;
    uint8_t encoderId;
    uint8_t motorId;
    uint32_t lastEncoderCount;
} BlindMotor;

/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void blind_motor_init(void);

void bm_stop_blind_moving(uint8_t blindId);
uint8_t bm_attempt_align_encoder(uint8_t encoderId);

#endif // BLIND_MOTOR_H
