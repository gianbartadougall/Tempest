#ifndef TEMPEST_CONFIG_H
#define TEMPEST_CONFIG_H

#include "motor.h"
#include "task_scheduler_1.h"
#include "task_scheduler_1_config.h"
#include "encoder.h"
#include "ambient_light_sensor.h"
#include "led.h"
#include "utilities.h"
#include "piezo_buzzer.h"

enum Mode { MANUAL, DAY_LIGHT, CONFIGURE_SETINGS };

enum TempestFunctions {
    FUNC_ID_SWITCH_BLIND_1_MODE_TO_DAYLIGHT,
    FUNC_ID_SWITCH_BLIND_2_MODE_TO_DAYLIGHT,
    FUNC_ID_BLINK_RED_LED,
    FUNC_ID_BLINK_ORANGE_LED,
    FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND,
};

const struct Task1 switchBlind1ToDayLightModeTask = {
    .delay      = 5000,
    .functionId = FUNC_ID_SWITCH_BLIND_1_MODE_TO_DAYLIGHT,
    .group      = TEMPEST_GROUP,
    .nextTask   = NULL,
};

const struct Task1 switchBlind2ToDayLightModeTask = {
    .delay      = 5000,
    .functionId = FUNC_ID_SWITCH_BLIND_2_MODE_TO_DAYLIGHT,
    .group      = TEMPEST_GROUP,
    .nextTask   = NULL,
};

struct Task1 blinkRedLedTask = {
    .delay      = 1000,
    .functionId = FUNC_ID_BLINK_RED_LED,
    .group      = TEMPEST_GROUP,
    .nextTask   = &blinkRedLedTask,
};

struct Task1 BlinkOrangeLedTask = {
    .delay      = 1000,
    .functionId = FUNC_ID_BLINK_ORANGE_LED,
    .group      = TEMPEST_GROUP,
    .nextTask   = &BlinkOrangeLedTask,
};

struct Task1 configSettingsSoundTask = {
    .delay      = 3000,
    .functionId = FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND,
    .group      = TEMPEST_GROUP,
    .nextTask   = &configSettingsSoundTask,
};

typedef struct Blind {
    const uint8_t motorId;
    const uint8_t encoderId;
    const uint8_t alSensorId;
    const uint8_t ledId;
    const Task1 switchToDayLightModeTask;
    Task1* blinkLedTask;
    Task1* configSettingsSoundTask;
    uint8_t mode;
    uint8_t previousMode;
    uint8_t status;
} Blind;

/* Private Variable Declarations */
Blind Blind1 = {
    .motorId                  = MOTOR_1_ID,
    .encoderId                = ENCODER_1_ID,
    .alSensorId               = AL_SENSOR_1_ID,
    .ledId                    = LED_RED_ID,
    .switchToDayLightModeTask = switchBlind1ToDayLightModeTask,
    .blinkLedTask             = &blinkRedLedTask,
    .configSettingsSoundTask  = &configSettingsSoundTask,
    .mode                     = MANUAL,
    .previousMode             = MANUAL,
    .status                   = DISCONNECTED,
};

Blind Blind2 = {
    .motorId                  = MOTOR_2_ID,
    .encoderId                = ENCODER_2_ID,
    .alSensorId               = AL_SENSOR_2_ID,
    .ledId                    = LED_ORANGE_ID,
    .switchToDayLightModeTask = switchBlind2ToDayLightModeTask,
    .blinkLedTask             = &BlinkOrangeLedTask,
    .configSettingsSoundTask  = &configSettingsSoundTask,
    .mode                     = MANUAL,
    .previousMode             = MANUAL,
    .status                   = DISCONNECTED,
};

#define MAX_NUM_BLINDS 2
Blind* blinds[MAX_NUM_BLINDS] = {&Blind1, &Blind2};

#endif // TEMPEST_CONFIG_H