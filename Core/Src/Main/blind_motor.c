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
#include "task_scheduler_1.h"
#include "encoder.h"
#include "utilities.h"

/* Private STM Includes */

/* Private #defines */

/* Private Structures and Enumerations */

enum BlindMotorEnums {
    FUNC_ID_TIMEOUT_REACHED,
    FUNC_ID_CONFIRM_ENCODER_1_IN_OPERATION,
    FUNC_ID_CONFIRM_ENCODER_2_IN_OPERATION,
    FUNC_ID_PRINT_TIMER_COUNT,
};

enum BlindMotorModes {
    BM_UPDATING_ENCODER,
    BM_NORMAL,
};

const struct Task1 TimeoutTask = {
    .delay      = 500,
    .functionId = FUNC_ID_TIMEOUT_REACHED,
    .group      = BLIND_MOTOR_GROUP,
    .nextTask   = NULL,
};

struct Task1 printTimerCount = {
    .delay      = 500,
    .functionId = FUNC_ID_PRINT_TIMER_COUNT,
    .group      = BLIND_MOTOR_GROUP,
    .nextTask   = &printTimerCount,
};

struct Task1 encoder1InOperation = {
    .delay      = 500,
    .functionId = FUNC_ID_CONFIRM_ENCODER_1_IN_OPERATION,
    .group      = BLIND_MOTOR_GROUP,
    .nextTask   = &encoder1InOperation,
};

struct Task1 encoder2InOperation = {
    .delay      = 500,
    .functionId = FUNC_ID_CONFIRM_ENCODER_2_IN_OPERATION,
    .group      = BLIND_MOTOR_GROUP,
    .nextTask   = &encoder2InOperation,
};

typedef struct BlindMotor {
    uint8_t blindId;
    uint8_t encoderId;
    uint8_t motorId;
    uint32_t lastEncoderCount;
    Task1* encoderCheckingTask;
    uint8_t mode;
} BlindMotor;

BlindMotor BlindMotors[NUM_BLINDS];

/* Private Variable Declarations */
extern uint32_t blindMotorFlag;
extern uint32_t encoderTaskFlags;

/* Private Function Prototypes */
uint8_t bm_search_blind_index(uint8_t blindId);

/* Public Functions */

void blind_motor_init(void) {

#ifdef ENCODER_MODULE_ENABLED
    encoder_init();

    BlindMotors[0].blindId             = BLIND_1_ID;
    BlindMotors[0].encoderId           = blind_get_encoder_id(BLIND_1_ID);
    BlindMotors[0].motorId             = blind_get_motor_id(BLIND_1_ID);
    BlindMotors[0].encoderCheckingTask = &encoder1InOperation;
    BlindMotors[0].lastEncoderCount    = 0;
    BlindMotors[0].mode                = BM_NORMAL;

    BlindMotors[1].blindId             = BLIND_2_ID;
    BlindMotors[1].encoderId           = blind_get_encoder_id(BLIND_2_ID);
    BlindMotors[1].motorId             = blind_get_motor_id(BLIND_2_ID);
    BlindMotors[1].encoderCheckingTask = &encoder2InOperation;
    BlindMotors[1].lastEncoderCount    = 0;
    BlindMotors[1].mode                = BM_NORMAL;

#endif
}

uint8_t bm_attempt_align_encoder(uint8_t blindId) {

    uint8_t index = bm_search_blind_index(blindId);

    if (index == INVALID_ID) {
        return FALSE;
    }

    // Start timeout count down then turn motor on
    ts_add_task_to_queue(&TimeoutTask);

    for (uint8_t i = 0; i < 20; i++) {

        // Turn the motor on for a short duration then check if encoder
        // reads high. Do this a maximum of
        motor_reverse(BlindMotors[index].motorId);
        HAL_Delay(10);
        motor_brake(BlindMotors[index].motorId);

        if (encoder_get_state(BlindMotors[index].encoderId) == PIN_HIGH) {
            debug_prints("CONNECTED\r\n");
            return CONNECTED;
        }
    }

    debug_prints("DISCONNECTED\r\n");
    return DISCONNECTED;
}

void bm_stop_blind_moving(uint8_t blindId) {

    uint8_t index = bm_search_blind_index(blindId);

    if (index == INVALID_ID) {
        return;
    }

    debug_prints("STOPPING MOTOR\r\n");
    motor_brake(BlindMotors[index].motorId);
    ts_cancel_running_task(BlindMotors[index].encoderCheckingTask);
    ts_cancel_running_task(&printTimerCount);
}

void bm_move_blind_up(uint8_t blindId) {

    uint8_t index = bm_search_blind_index(blindId);

    if (index == INVALID_ID) {
        return;
    }

    if (BlindMotors[index].mode == BM_NORMAL && encoder_at_max_height(BlindMotors[index].encoderId)) {
        debug_prints("Already at max height\r\n");
        return;
    }

    uint8_t encoderId = BlindMotors[index].encoderId;

    // Set the blind direction to moving down
    encoder_set_direction_up(encoderId);

    // Set current encoder count and start encoder checking
    BlindMotors[index].lastEncoderCount = encoder_get_count(encoderId);
    ts_add_task_to_queue(BlindMotors[index].encoderCheckingTask);
    ts_add_task_to_queue(&printTimerCount);

    motor_forward(BlindMotors[index].motorId);
}

void bm_move_blind_down(uint8_t blindId) {

    uint8_t index = bm_search_blind_index(blindId);

    if (index == INVALID_ID) {
        return;
    }

    if (BlindMotors[index].mode == BM_NORMAL && encoder_at_min_height(BlindMotors[index].encoderId)) {
        debug_prints("Already at min height\r\n");
        return;
    }

    uint8_t encoderId = BlindMotors[index].encoderId;

    // Set the blind direction to moving down
    encoder_set_direction_down(encoderId);

    // Set current encoder count and start encoder checking
    BlindMotors[index].lastEncoderCount = encoder_get_count(encoderId);
    ts_add_task_to_queue(BlindMotors[index].encoderCheckingTask);
    ts_add_task_to_queue(&printTimerCount);

    motor_reverse(BlindMotors[index].motorId);
}

uint8_t emergency_motor_stop(uint8_t blindId) {

    uint8_t index = bm_search_blind_index(blindId);

    if (index == INVALID_ID) {
        return TRUE;
    }

    if (BlindMotors[index].lastEncoderCount == encoder_get_count(BlindMotors[index].encoderId)) {
        return TRUE;
    }

    return FALSE;
}

void bm_process_internal_flags(void) {

    if (FLAG_IS_SET(blindMotorFlag, FUNC_ID_CONFIRM_ENCODER_1_IN_OPERATION)) {
        FLAG_CLEAR(blindMotorFlag, FUNC_ID_CONFIRM_ENCODER_1_IN_OPERATION);
        if (emergency_motor_stop(BLIND_1_ID) != FALSE) {
            bm_stop_blind_moving(BLIND_1_ID);
        }
    }

    if (FLAG_IS_SET(blindMotorFlag, FUNC_ID_CONFIRM_ENCODER_2_IN_OPERATION)) {
        FLAG_CLEAR(blindMotorFlag, FUNC_ID_CONFIRM_ENCODER_2_IN_OPERATION);
        if (emergency_motor_stop(BLIND_1_ID) != FALSE) {
            bm_stop_blind_moving(BLIND_2_ID);
        }
    }

    if (FLAG_IS_SET(blindMotorFlag, FUNC_ID_PRINT_TIMER_COUNT)) {
        FLAG_CLEAR(blindMotorFlag, FUNC_ID_PRINT_TIMER_COUNT);
        char m[60];
        sprintf(m, "TIM: %li\tCCR2: %li\t CCR3: %li\r\n", TIM1->CNT, TIM1->CCR2, TIM1->CCR3);
        debug_prints(m);
    }

    /****** START CODE BLOCK ******/
    // Description: Process the encoder flags that tell the motor when
    // it needs to stop

    if (FLAG_IS_SET(encoderTaskFlags, ENCODER_1_LIMIT_REACHED)) {
        FLAG_CLEAR(encoderTaskFlags, ENCODER_1_LIMIT_REACHED);
        bm_stop_blind_moving(BLIND_1_ID);
    }

    if (FLAG_IS_SET(encoderTaskFlags, ENCODER_2_LIMIT_REACHED)) {
        FLAG_CLEAR(encoderTaskFlags, ENCODER_2_LIMIT_REACHED);
        bm_stop_blind_moving(BLIND_2_ID);
    }

    /****** END CODE BLOCK ******/
}

void bm_set_new_min_height(uint8_t blindId) {

    uint8_t index = bm_search_blind_index(blindId);

    if (index == INVALID_ID) {
        return;
    }

    encoder_set_upper_bound_interrupt(BlindMotors[index].encoderId);
}

void bm_set_new_max_height(uint8_t blindId) {

    uint8_t index = bm_search_blind_index(blindId);

    if (index == INVALID_ID) {
        return;
    }

    // The maximum height sets the zero point for the encoder
    encoder_set_lower_bound_interrupt(BlindMotors[index].encoderId);
}

uint8_t bm_min_max_heights_are_valid(uint8_t blindId) {

    uint8_t index = bm_search_blind_index(blindId);

    if (index == INVALID_ID) {
        return FALSE;
    }

    // Heights are only valid if min height < max height and the current
    // encoder count is in between the bounds
    uint8_t encoderId             = BlindMotors[index].encoderId;
    uint32_t lowerBoundInterrupt  = encoder_get_lower_bound_interrupt(encoderId);
    uint32_t uppperBoundInterrupt = encoder_get_upper_bound_interrupt(encoderId);
    uint32_t currentCount         = encoder_get_count(encoderId);

    if (lowerBoundInterrupt >= uppperBoundInterrupt) {
        return FALSE;
    }

    if ((currentCount < lowerBoundInterrupt) || (currentCount > uppperBoundInterrupt)) {
        return FALSE;
    }

    BlindMotors[index].mode = BM_NORMAL;
    encoder_enable_interrupts(BlindMotors[index].encoderId);
    return TRUE;
}

void bm_set_mode_update_encoder_settings(uint8_t blindId) {

    uint8_t index = bm_search_blind_index(blindId);

    if (index == INVALID_ID) {
        return;
    }

    BlindMotors[index].mode = BM_UPDATING_ENCODER;
    encoder_disable_interrupts(BlindMotors[index].encoderId);
}

/* Private Functions */

uint8_t bm_search_blind_index(uint8_t blindId) {

    for (uint8_t i = 0; i < NUM_BLINDS; i++) {
        if (BlindMotors[i].blindId == blindId) {
            return i;
        }
    }

    return INVALID_ID;
}