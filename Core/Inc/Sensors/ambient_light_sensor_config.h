#ifndef AMBIENT_LIGHT_SENSOR_CONFIG_H
#define AMBIENT_LIGHT_SENSOR_CONFIG_H

#include "ambient_light_sensor.h"
#include "hardware_config.h"

/* Includes for repeated recipes */
#define NUM_AL_SENSORS 2

typedef struct AmbientLightSensor {
    GPIO_TypeDef* port;
    const uint32_t pin;
    const uint8_t id;
} AmbientLightSensor;

/* Initialise Ambient light sensor */

#if (VERSION_MAJOR == 0)

const AmbientLightSensor alSensor1 = {
    .id   = AL_SENSOR_1_ID,
    .port = HC_ALS_PORT_1,
    .pin  = HC_ALS_PIN_1,
};

const AmbientLightSensor alSensor2 = {
    .id   = AL_SENSOR_2_ID,
    .port = HC_ALS_PORT_2,
    .pin  = HC_ALS_PIN_2,
};

#endif

AmbientLightSensor alSensors[NUM_AL_SENSORS] = {alSensor1, alSensor2};

#endif // AMBIENT_LIGHT_SENSOR_CONFIG_H