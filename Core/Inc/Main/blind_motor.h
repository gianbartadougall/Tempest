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

/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void blind_motor_init(void);

void bm_stop_blind_moving(uint8_t blindId);
void bm_move_blind_up(uint8_t blindId);
void bm_move_blind_down(uint8_t blindId);
void bm_process_internal_flags(void);
void bm_set_new_min_height(uint8_t blindId);
void bm_set_new_max_height(uint8_t blindId);
void bm_set_mode_update_encoder_settings(uint8_t blindId);
uint8_t bm_min_max_heights_are_valid(uint8_t blindId);
uint8_t bm_attempt_align_encoder(uint8_t blindId);

#endif // BLIND_MOTOR_H
