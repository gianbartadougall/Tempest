#ifndef TASK_SCHEDULER_1_CONFIG_H
#define TASK_SCHEDULER_1_CONFIG_H

#include "task_scheduler_1.h"
#include "task_scheduler_1_id_config.h"

/* Includes for function pointers used by recipes that repeat */

/**
 * @brief To read the ambient light sensor the following steps need to
 * occur
 *      1. Set the data line low to discharge capacitor
 *      2. Delay 1 second to ensure capacitor is discharged
 *      3. Set data line to high impedence
 *      4. Delay 59 seconds to let capacitor charge from ambient light
 *      5. Set data line to input mode
 *      6. Wait 1ms for IDR to update
 *      6. Update state based on the IDR of the data pin
 *      7. Set data line to high
 *      8. Wait for 1 second for capacitor to charge
 *      9. Update state based on input (if sensor is connected input = 1 else 0)
 */

/******************** Ambient Light Sensor 1 Process ********************/
struct Task1 alSensor1DischargeCapacitor;

struct Task1 alSensor1ConfirmSensorIsConnected = {
    .processId  = TS_ID_READ_AL_SENSOR_1,
    .delay      = 300,
    .functionId = 4,
    .group      = AMBIENT_LIGHT_SENSOR_GROUP,
    .nextTask   = &alSensor1DischargeCapacitor,
};

struct Task1 alSensor1chargeCapacitor = {
    .processId  = TS_ID_READ_AL_SENSOR_1,
    .delay      = 20,
    .functionId = 3,
    .group      = AMBIENT_LIGHT_SENSOR_GROUP,
    .nextTask   = &alSensor1ConfirmSensorIsConnected,
};

struct Task1 alSensor1ReadCapacitorCharge = {
    .processId  = TS_ID_READ_AL_SENSOR_1,
    .delay      = 58000,
    .functionId = 2,
    .group      = AMBIENT_LIGHT_SENSOR_GROUP,
    .nextTask   = &alSensor1chargeCapacitor,
};

struct Task1 alSensor1SetModeAnalogue = {
    .processId  = TS_ID_READ_AL_SENSOR_1,
    .delay      = 300,
    .functionId = 1,
    .group      = AMBIENT_LIGHT_SENSOR_GROUP,
    .nextTask   = &alSensor1ReadCapacitorCharge,
};

struct Task1 alSensor1DischargeCapacitor = {
    .processId  = TS_ID_READ_AL_SENSOR_1,
    .delay      = 50,
    .functionId = 0,
    .group      = AMBIENT_LIGHT_SENSOR_GROUP,
    .nextTask   = &alSensor1SetModeAnalogue,
};
/************************************************************************/

/******************** Ambient Light Sensor 2 Process ********************/
struct Task1 alSensor2DischargeCapacitor;

struct Task1 alSensor2ConfirmSensorIsConnected = {
    .processId  = TS_ID_READ_AL_SENSOR_2,
    .delay      = 300,
    .functionId = 9,
    .group      = AMBIENT_LIGHT_SENSOR_GROUP,
    .nextTask   = &alSensor2DischargeCapacitor,
};

struct Task1 alSensor2chargeCapacitor = {
    .processId  = TS_ID_READ_AL_SENSOR_2,
    .delay      = 20,
    .functionId = 8,
    .group      = AMBIENT_LIGHT_SENSOR_GROUP,
    .nextTask   = &alSensor2ConfirmSensorIsConnected,
};

struct Task1 alSensor2ReadCapacitorCharge = {
    .processId  = TS_ID_READ_AL_SENSOR_2,
    .delay      = 58000,
    .functionId = 7,
    .group      = AMBIENT_LIGHT_SENSOR_GROUP,
    .nextTask   = &alSensor2chargeCapacitor,
};

struct Task1 alSensor2SetModeAnalogue = {
    .processId  = TS_ID_READ_AL_SENSOR_2,
    .delay      = 300,
    .functionId = 6,
    .group      = AMBIENT_LIGHT_SENSOR_GROUP,
    .nextTask   = &alSensor2ReadCapacitorCharge,
};

struct Task1 alSensor2DischargeCapacitor = {
    .processId  = TS_ID_READ_AL_SENSOR_2,
    .delay      = 50,
    .functionId = 5,
    .group      = AMBIENT_LIGHT_SENSOR_GROUP,
    .nextTask   = &alSensor2SetModeAnalogue,
};
/************************************************************************/

#endif // TASK_SCHEDULER_1_CONFIG_H