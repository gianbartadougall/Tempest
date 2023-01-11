/**
 * @file blind_motor.h
 * @author Gian Barta-Dougall
 * @brief This file is a wrapper file which handles the logic for the rotary
 * encoder coupled with the motor. The purpose of this file is to abstract
 * away the intracacies of operating the motor and encoder together. This wrapper
 * handles things like ensuring the blind cannot go past it's maximum and minimum
 * heights during normal operation
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef BLIND_MOTOR_H
#define BLIND_MOTOR_H

/* Public Includes */
#include "task_scheduler_1.h"
#include "motor.h"

/* Public STM Includes */
#include "stm32l4xx.h"

/* Public #defines */
#define BLIND_MOTOR_ID_OFFSET 76
#define BLIND_MOTOR_1_ID      (0 + BLIND_MOTOR_ID_OFFSET)
#define BLIND_MOTOR_2_ID      (1 + BLIND_MOTOR_ID_OFFSET)
#define NUM_BLIND_MOTORS      NUM_BLINDS

#define BLIND_UP   MOTOR_FORWARD
#define BLIND_DOWN MOTOR_REVERSE

/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void blind_motor_init(void);

/**
 * @brief Turns the motor attached to the given blind off
 *
 * @param blindId The ID to the blind to stop moving
 */
void bm_stop_blind_moving(uint8_t blindId);

/**
 * @brief Turns the motor connected to the given blind on in
 * the upwards direction
 *
 * @param blindId The ID of the blind to move upwards
 * @param motorDirection The direction to move the blind in
 */
void bm_move_blind(uint8_t blindId, enum MotorDirection motorDirection);

/**
 * @brief Processes any internal flags that are set through interrupts.
 * This function should be called repeatedly as quickly as possible to
 * ensure any flags that are raised are dealt ASAP
 */
void bm_process_internal_flags(void);

/**
 * @brief Sets the new minimum height of the blind
 *
 * @param blindId The ID of the blind to set the new minimum height of
 */
void bm_set_new_min_height(uint8_t blindId);

/**
 * @brief Sets the new maximum height of the blind
 *
 * @param blindId The ID of the blind to set the new maximum height of
 */
void bm_set_new_max_height(uint8_t blindId);

/**
 * @brief Settings the mode of the blind to 'update encoder settings' means
 * the user is currently in a mode where the minimum and maximum heights
 * of the blind can be adjusted. This means the blinds can go outside their
 * normal range of operation whilst in this mode.
 *
 * @param blindId The ID of the blind who's encoder settings are being updated
 */
void bm_set_mode_update_encoder_settings(uint8_t blindId);

/**
 * @brief Checks whether the maximum height is > minimum height. If this is
 * true then the min and max heights are valid and the mode of the blind is
 * automatically reset to 'normal mode'
 *
 * @param blindId The ID of the blind who's min and max heights are being checked
 * @return uint8_t TRUE if the blind min/max heights are valid else FALSE
 */
uint8_t bm_min_max_heights_are_valid(uint8_t blindId);

/**
 * @brief Checks whether the blind motor is properly connected. This is done
 * by checking if the encoder can detect the blind
 *
 * @param blindMotorId
 */
uint8_t bm_probe_connection(uint8_t blindMotorId);

uint8_t bm_blind_at_max_height(uint8_t blindMotorId);
uint8_t bm_blind_at_min_height(uint8_t blindMotorId);
uint16_t bm_get_height(uint8_t blindMotorId);

#endif // BLIND_MOTOR_H
