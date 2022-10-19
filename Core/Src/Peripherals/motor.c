/**
 * @file motor.c
 * @author Gian Barta-Dougall
 * @brief System file for motor
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "motor_config.h"
#include "utilities.h"

/* Private STM Includes */

/* Private #defines */
#define PORT_A                        0
#define PORT_B                        1
#define SET_PORT_HIGH(mIndex, pIndex) (motors[mIndex].ports[pIndex]->BSRR |= (0x01 << motors[mIndex].pins[pIndex]))
#define SET_PORT_LOW(mIndex, pIndex)  (motors[mIndex].ports[pIndex]->BSRR |= (0x10000 << motors[mIndex].pins[pIndex]))

#define ID_INVALID(id) ((id < MOTOR_ID_OFFSET) || (id > (NUM_MOTORS - 1 + MOTOR_ID_OFFSET)))

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */

/* Public Functions */

void motor_init(void) {}

void motor_forward(uint8_t motorId) {

    if (ID_INVALID(motorId)) {
        return;
    }
    uint8_t index = motorId - MOTOR_ID_OFFSET;
    SET_PIN_HIGH(motors[index].ports[0], motors[index].pins[0]);
    SET_PIN_LOW(motors[index].ports[1], motors[index].pins[1]);
}

void motor_reverse(uint8_t motorId) {

    if (ID_INVALID(motorId)) {
        debug_prints("returning\r\n");
        return;
    }

    debug_prints("Setting motor reverse\r\n");
    uint8_t index = motorId - MOTOR_ID_OFFSET;
    SET_PIN_LOW(motors[index].ports[0], motors[index].pins[0]);
    SET_PIN_HIGH(motors[index].ports[1], motors[index].pins[1]);
}

void motor_brake(uint8_t motorId) {

    if (ID_INVALID(motorId)) {
        return;
    }

    uint8_t index = motorId - MOTOR_ID_OFFSET;
    SET_PIN_HIGH(motors[index].ports[0], motors[index].pins[0]);
    SET_PIN_HIGH(motors[index].ports[1], motors[index].pins[1]);
}

void motor_stop(uint8_t motorId) {

    if (ID_INVALID(motorId)) {
        return;
    }

    uint8_t index = motorId - MOTOR_ID_OFFSET;
    SET_PIN_LOW(motors[index].ports[0], motors[index].pins[0]);
    SET_PIN_LOW(motors[index].ports[1], motors[index].pins[1]);
}

/* Private Functions */