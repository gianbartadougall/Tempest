/**
 * @file blind_motor.c
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
/* Public Includes */

/* Private Includes */
#include "blind_motor.h"
#include "motor.h"
#include "blind.h"
#include "task_scheduler.h"
#include "encoder.h"
#include "utilities.h"

/* Private STM Includes */

/* Private #defines */

/* Private Structures and Enumerations */

BlindMotor BlindMotors[NUM_BLINDS];

/* Private Variable Declarations */

/* Private Function Prototypes */

/* Public Functions */

void blind_motor_init(void) {

    BlindMotors[0].blindId          = BLIND_1_ID;
    BlindMotors[0].encoderId        = blind_get_encoder_id(BLIND_1_ID);
    BlindMotors[0].motorId          = blind_get_motor_id(BLIND_1_ID);
    BlindMotors[0].lastEncoderCount = 0;

    BlindMotors[1].blindId          = BLIND_2_ID;
    BlindMotors[1].encoderId        = blind_get_encoder_id(BLIND_2_ID);
    BlindMotors[1].motorId          = blind_get_motor_id(BLIND_2_ID);
    BlindMotors[1].lastEncoderCount = 0;
}

uint8_t bm_attempt_align_encoder(uint8_t encoderId) {
    return TRUE;
}

void bm_stop_blind_moving(uint8_t blindId) {

    for (uint8_t i = 0; i < NUM_BLINDS; i++) {

        if (blindId != BlindMotors[i].blindId) {
            continue;
        }

        motor_brake(BlindMotors[i].motorId);
        blind_cancel_encoder_checking_task(blindId);
    }
}

void bm_move_blind_to_max_height(uint8_t blindId) {}

void bm_move_blind_to_min_height(uint8_t blindId) {

    for (uint8_t i = 0; i < NUM_BLINDS; i++) {
        // Set the blind direction to moving down
        encoder_set_direction_down(BlindMotors[i].encoderId);

        // Store the current count of the encoder to
    }
}

void bm_move_blind_up(uint8_t blindId) {}

void bm_move_blind_down(uint8_t blindId) {}

/* Private Functions */
