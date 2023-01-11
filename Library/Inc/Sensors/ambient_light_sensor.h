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

#define AL_SENSOR_ID_OFFSET 43
#define AL_SENSOR_1_ID      (0 + AL_SENSOR_ID_OFFSET)
#define AL_SENSOR_2_ID      (1 + AL_SENSOR_ID_OFFSET)

/**
 * @brief Processes internal flags that call private functions
 * within the al sensor .c file which update the status of the
 * sensor and check whether the sensor is connected
 */
void al_sensor_process_internal_flags(void);

/**
 * @brief Reads the current status of a given AL sensor sensor
 *
 * @param alSensorId The id of the ambient light sensor to read
 * the status of
 * @return uint8_t True if ambient light was detected, False if
 * no light was detected and DISCONNECTED if the sensor could not
 * be read from
 */
uint8_t al_sensor_light_found(uint8_t alSensorId);

/**
 * @brief Checks the current status of the give ambient light sensor
 *
 * @param alSensorId The id of the ambient light sensor to get the status of
 * @return uint8_t CONNECTED if the sensor is connected else DISCONNECTED
 */
uint8_t al_sensor_status(uint8_t alSensorId);

#endif // AMBIENT_LIGHT_SENSOR_H