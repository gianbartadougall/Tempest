#ifndef MOTOR_CONFIG_H
#define MOTOR_CONFIG_H

/* Public Includes */
#include "hardware_config.h"
#include "version_config.h"
#include "motor.h"

typedef struct Motor {
    const uint8_t id;
    GPIO_TypeDef* ports[2];
    const uint32_t pins[2];
} Motor;

/* Initialise Motors */

#if (VERSION_MAJOR == 0)

const Motor rollerBlindMotor1 = {
    .id    = ROLLER_BLIND_MOTOR_1,
    .ports = {HC_MOTOR_PORT_1, HC_MOTOR_PORT_2},
    .pins  = {HC_MOTOR_PIN_1, HC_MOTOR_PIN_2},
};

const Motor rollerBlindMotor2 = {
    .id    = ROLLER_BLIND_MOTOR_2,
    .ports = {HC_MOTOR_PORT_3, HC_MOTOR_PORT_4},
    .pins  = {HC_MOTOR_PIN_3, HC_MOTOR_PIN_4},
};

#endif

#define NUM_MOTORS 2
Motor motors[NUM_MOTORS] = {rollerBlindMotor1, rollerBlindMotor2};

#endif // MOTOR_CONFIG_H