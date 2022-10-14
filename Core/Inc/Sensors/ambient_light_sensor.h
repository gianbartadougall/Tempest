#ifndef AMBIENT_LIGHT_SENSOR_H
#define AMBIENT_LIGHT_SENSOR_H

#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"

/* Public Enums */
enum AmbientLightSensorFunctions {
    ALS1_DISCHARGE_CAPACITOR,
    ALS1_RECORD_AMBIENT_LIGHT,
    ALS1_READ_AMBIENT_LIGHT,
    ALS1_CHARGE_CAPACITOR,
    ALS1_CONFIRM_CONNECTION,
    ALS2_DISCHARGE_CAPACITOR,
    ALS2_RECORD_AMBIENT_LIGHT,
    ALS2_READ_AMBIENT_LIGHT,
    ALS2_CHARGE_CAPACITOR,
    ALS2_CONFIRM_CONNECTION,
};

#define AL_SENSOR_OFFSET 43
#define AL_SENSOR_1      (0 + AL_SENSOR_OFFSET)
#define AL_SENSOR_2      (1 + AL_SENSOR_OFFSET)

void al_sensor_process_flags(void);
uint8_t al_sensor_read_status(uint8_t alSensorId);

#endif // AMBIENT_LIGHT_SENSOR_H