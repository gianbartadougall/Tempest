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
#include "blind.h"
#include "task_scheduler_1.h"
#include "encoder.h"
#include "utilities.h"
#include "piezo_buzzer.h"

/* Private STM Includes */

/* Private #defines */
#define ASSERT_VALID_BLIND_MOTOR_ID(id)                                                        \
    do {                                                                                       \
        if ((id < BLIND_MOTOR_ID_OFFSET) || (id > (NUM_BLINDS - 1 + BLIND_MOTOR_ID_OFFSET))) { \
            char msg[100];                                                                     \
            sprintf(msg, "Invalid ID: File %s line number %d\r\n", __FILE__, __LINE__);        \
            log_prints(msg);                                                                   \
            return;                                                                            \
        }                                                                                      \
    } while (0)

#define ASSERT_VALID_BLIND_MOTOR_ID_RETVAL(id, retval)                                         \
    do {                                                                                       \
        if ((id < BLIND_MOTOR_ID_OFFSET) || (id > (NUM_BLINDS - 1 + BLIND_MOTOR_ID_OFFSET))) { \
            char msg[100];                                                                     \
            sprintf(msg, "Invalid ID: File %s line number %d\r\n", __FILE__, __LINE__);        \
            log_prints(msg);                                                                   \
            return retval;                                                                     \
        }                                                                                      \
    } while (0)

#define BLIND_MOTOR_ID_TO_INDEX(id) (id - BLIND_MOTOR_ID_OFFSET)

#define SET_MIN_HEIGHT_SOUND SOUND1
#define SET_MAX_HEIGHT_SOUND SOUND1

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
    uint8_t id;
    uint8_t encoderId;
    uint8_t motorId;
    uint32_t lastEncoderCount;
    Task1* encoderCheckingTask;
    uint8_t mode;
} BlindMotor;

BlindMotor BlindMotor1 = {
    .id                  = BLIND_MOTOR_1_ID,
    .encoderId           = ENCODER_1_ID,
    .motorId             = MOTOR_1_ID,
    .lastEncoderCount    = 0,
    .encoderCheckingTask = &encoder1InOperation,
    .mode                = DISCONNECTED,
};

BlindMotor BlindMotor2 = {
    .id                  = BLIND_MOTOR_2_ID,
    .encoderId           = ENCODER_2_ID,
    .motorId             = MOTOR_2_ID,
    .lastEncoderCount    = 0,
    .encoderCheckingTask = &encoder2InOperation,
    .mode                = DISCONNECTED,
};

BlindMotor* BlindMotors[NUM_BLINDS] = {&BlindMotor1, &BlindMotor2};

/* Private Variable Declarations */
extern uint32_t blindMotorFlag;
extern uint32_t encoderTaskFlags;

/* Private Function Prototypes */
uint8_t bm_attempt_align_encoder(uint8_t blindId);
uint8_t emergency_motor_stop(uint8_t blindMotorId);

/* Public Functions */

void blind_motor_init(void) {

    // Initialise the encoders
    encoder_init();
}

uint8_t bm_probe_connection(uint8_t blindMotorId) {

    ASSERT_VALID_BLIND_MOTOR_ID_RETVAL(blindMotorId, DISCONNECTED);

    uint8_t index = BLIND_MOTOR_ID_TO_INDEX(blindMotorId);

    if (encoder_get_state(BlindMotors[index]->encoderId) == PIN_HIGH) {
        return CONNECTED;
    }

    if (bm_attempt_align_encoder(blindMotorId) == TRUE) {
        return CONNECTED;
    }

    return DISCONNECTED;
}

void bm_stop_blind_moving(uint8_t blindMotorId) {

    ASSERT_VALID_BLIND_MOTOR_ID(blindMotorId);
    uint8_t index = BLIND_MOTOR_ID_TO_INDEX(blindMotorId);

    // log_prints("STOPPING MOTOR\r\n");
    motor_brake(BlindMotors[index]->motorId);
    ts_cancel_running_task(BlindMotors[index]->encoderCheckingTask);
}

void bm_move_blind(uint8_t blindMotorId, uint8_t motorDirection) {

    ASSERT_VALID_BLIND_MOTOR_ID(blindMotorId);
    uint8_t index = BLIND_MOTOR_ID_TO_INDEX(blindMotorId);

    uint8_t encoderId = BlindMotors[index]->encoderId;
    if (BlindMotors[index]->mode == BM_NORMAL) {

        if (motorDirection == MOTOR_FORWARD && encoder_at_max_height(encoderId) == TRUE) {
            return;
        }

        if (motorDirection == MOTOR_REVERSE && encoder_at_min_height(encoderId) == TRUE) {
            return;
        }
    }

    uint8_t motorState = motor_get_state(BlindMotors[index]->motorId);

    // Nothing to do if motor already in the desired direction
    if (motorState == motorDirection) {
        return;
    }

    // If the desired motor direction is opposite to the current motor direction
    // stop the blind
    if (motorState == MOTOR_FORWARD || motorState == MOTOR_REVERSE) {
        bm_stop_blind_moving(blindMotorId);
        return;
    }

    // Blind needs to move either up or down. Update the last count and begin
    // task to ensure encoder is updating correctly
    BlindMotors[index]->lastEncoderCount = encoder_get_count(encoderId);
    ts_add_task_to_queue(BlindMotors[index]->encoderCheckingTask);

    // Move the motor in the desired direction
    if (motorDirection == MOTOR_FORWARD) {
        encoder_set_direction_up(encoderId);
        motor_forward(BlindMotors[index]->motorId);
    }

    if (motorDirection == MOTOR_REVERSE) {
        encoder_set_direction_down(encoderId);
        motor_reverse(BlindMotors[index]->motorId);
    }
}

void bm_process_internal_flags(void) {

    if (FLAG_IS_SET(blindMotorFlag, FUNC_ID_CONFIRM_ENCODER_1_IN_OPERATION)) {
        FLAG_CLEAR(blindMotorFlag, FUNC_ID_CONFIRM_ENCODER_1_IN_OPERATION);
        if (BlindMotor1.lastEncoderCount == encoder_get_count(BlindMotor1.encoderId)) {
            bm_stop_blind_moving(BlindMotor1.id);
        }
    }

    if (FLAG_IS_SET(blindMotorFlag, FUNC_ID_CONFIRM_ENCODER_2_IN_OPERATION)) {
        FLAG_CLEAR(blindMotorFlag, FUNC_ID_CONFIRM_ENCODER_2_IN_OPERATION);

        if (BlindMotor2.lastEncoderCount == encoder_get_count(BlindMotor2.encoderId)) {
            bm_stop_blind_moving(BlindMotor2.id);
        }
    }

    if (FLAG_IS_SET(blindMotorFlag, FUNC_ID_PRINT_TIMER_COUNT)) {
        FLAG_CLEAR(blindMotorFlag, FUNC_ID_PRINT_TIMER_COUNT);
        char m[60];
        sprintf(m, "TIM: %li\tCCR2: %li\t CCR3: %li\r\n", TIM1->CNT, TIM1->CCR2, TIM1->CCR3);
        log_prints(m);
    }

    /****** START CODE BLOCK ******/
    // Description: Process the encoder flags that tell the motor when
    // it needs to stop

    if (FLAG_IS_SET(encoderTaskFlags, ENCODER_1_LIMIT_REACHED)) {
        FLAG_CLEAR(encoderTaskFlags, ENCODER_1_LIMIT_REACHED);
        bm_stop_blind_moving(BlindMotor1.id);
    }

    if (FLAG_IS_SET(encoderTaskFlags, ENCODER_2_LIMIT_REACHED)) {
        FLAG_CLEAR(encoderTaskFlags, ENCODER_2_LIMIT_REACHED);
        bm_stop_blind_moving(BlindMotor2.id);
    }

    /****** END CODE BLOCK ******/
}

void bm_set_new_min_height(uint8_t blindMotorId) {

    ASSERT_VALID_BLIND_MOTOR_ID(blindMotorId);
    uint8_t index = BLIND_MOTOR_ID_TO_INDEX(blindMotorId);

    piezo_buzzer_play_sound(SET_MIN_HEIGHT_SOUND);
    encoder_set_upper_bound_interrupt(BlindMotors[index]->encoderId);
    char m[60];
    sprintf(m, "Min height set to: %li\r\n", encoder_get_upper_bound_interrupt(BlindMotors[index]->encoderId));
    log_prints(m);
}

void bm_set_new_max_height(uint8_t blindMotorId) {

    ASSERT_VALID_BLIND_MOTOR_ID(blindMotorId);
    uint8_t index = BLIND_MOTOR_ID_TO_INDEX(blindMotorId);

    // The maximum height sets the zero point for the encoder
    piezo_buzzer_play_sound(SET_MAX_HEIGHT_SOUND);
    encoder_set_lower_bound_interrupt(BlindMotors[index]->encoderId);

    // char m[60];
    // sprintf(m, "Max height set to: %li\r\n", encoder_get_lower_bound_interrupt(BlindMotors[index]->encoderId));
    // log_prints(m);
}

uint8_t bm_min_max_heights_are_valid(uint8_t blindMotorId) {

    ASSERT_VALID_BLIND_MOTOR_ID_RETVAL(blindMotorId, FALSE);

    uint8_t index = BLIND_MOTOR_ID_TO_INDEX(blindMotorId);

    // Heights are only valid if min height < max height and the current
    // encoder count is in between the bounds
    uint8_t encoderId             = BlindMotors[index]->encoderId;
    uint32_t lowerBoundInterrupt  = encoder_get_lower_bound_interrupt(encoderId);
    uint32_t uppperBoundInterrupt = encoder_get_upper_bound_interrupt(encoderId);
    uint32_t currentCount         = encoder_get_count(encoderId);

    if (lowerBoundInterrupt >= uppperBoundInterrupt) {
        return FALSE;
    }

    if ((currentCount < lowerBoundInterrupt) || (currentCount > uppperBoundInterrupt)) {
        return FALSE;
    }

    BlindMotors[index]->mode = BM_NORMAL;
    encoder_enable_interrupts(BlindMotors[index]->encoderId);
    return TRUE;
}

void bm_set_mode_update_encoder_settings(uint8_t blindMotorId) {

    ASSERT_VALID_BLIND_MOTOR_ID(blindMotorId);

    uint8_t index = BLIND_MOTOR_ID_TO_INDEX(blindMotorId);

    BlindMotors[index]->mode = BM_UPDATING_ENCODER;
    encoder_disable_interrupts(BlindMotors[index]->encoderId);
}

uint8_t bm_blind_at_max_height(uint8_t blindMotorId) {

    ASSERT_VALID_BLIND_MOTOR_ID_RETVAL(blindMotorId, TRUE);
    uint8_t index = BLIND_MOTOR_ID_TO_INDEX(blindMotorId);

    return encoder_at_max_height(BlindMotors[index]->encoderId);
}

uint8_t bm_blind_at_min_height(uint8_t blindMotorId) {

    ASSERT_VALID_BLIND_MOTOR_ID_RETVAL(blindMotorId, TRUE);
    uint8_t index = BLIND_MOTOR_ID_TO_INDEX(blindMotorId);

    return encoder_at_min_height(BlindMotors[index]->encoderId);
}

uint16_t bm_get_height(uint8_t blindMotorId) {
    return encoder_get_count(BlindMotors[blindMotorId]->encoderId);
}

/* Private Functions */

/**
 * @brief Attempts to align the gears of the encoder with the encoder sensor.
 * This is done by moving the motor slowly for a short duration of time and
 * checking whether the encoder detects the gear moving. The motor is stopped
 * either as soon as the gear is detected or as soon as the timeout finishes.
 * The timeout is a short duration that ensures the motor will stop if a gear
 * has not been detected soon enough after the motor has begun turning
 *
 * @param blindId The ID of the blind to who's encoder is attempting to be alligned
 * @return uint8_t TRUE if the encoder could be alligned else FALSE
 */
uint8_t bm_attempt_align_encoder(uint8_t blindMotorId) {

    // Don't need to check the ID because this is a private function and all
    // id's are checked by public functions first
    uint8_t index = BLIND_MOTOR_ID_TO_INDEX(blindMotorId);

    // Start timeout count down then turn motor on
    ts_add_task_to_queue(&TimeoutTask);

    for (uint8_t i = 0; i < 20; i++) {

        // Turn the motor on for a short duration then check if encoder
        // reads high. Do this a maximum of
        motor_reverse(BlindMotors[index]->motorId);
        HAL_Delay(10);
        motor_brake(BlindMotors[index]->motorId);

        if (encoder_get_state(BlindMotors[index]->encoderId) == PIN_HIGH) {
            log_prints("CONNECTED\r\n");
            return CONNECTED;
        }
    }

    log_prints("DISCONNECTED\r\n");
    return DISCONNECTED;
}