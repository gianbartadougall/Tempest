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

const Motor Motor1 = {
    .id    = MOTOR_1_ID,
    .ports = {HC_MOTOR_PORT_1, HC_MOTOR_PORT_2},
    .pins  = {HC_MOTOR_PIN_1, HC_MOTOR_PIN_2},
};

const Motor Motor2 = {
    .id    = MOTOR_2_ID,
    .ports = {HC_MOTOR_PORT_3, HC_MOTOR_PORT_4},
    .pins  = {HC_MOTOR_PIN_3, HC_MOTOR_PIN_4},
};

#endif

#define NUM_MOTORS 2

Motor motors[NUM_MOTORS] = {Motor1, Motor2};

#endif // MOTOR_CONFIG_H