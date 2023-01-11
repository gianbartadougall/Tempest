#ifndef USER_INTERFACE_CONFIG_H
#define USER_INTERFACE_CONFIG_H

#include "hardware_config.h"
#include "user_interface.h"
#include "led.h"
#include "piezo_buzzer.h"
#include "blind.h"

/* Initialise LEDs */

#define NUM_INTERFACES 2

struct Task1 blinkRedLedTask = {
    .delay      = 1000,
    .functionId = FUNC_ID_BLINK_RED_LED,
    .group      = BLIND_GROUP,
    .nextTask   = &blinkRedLedTask,
};

struct Task1 BlinkOrangeLedTask = {
    .delay      = 1000,
    .functionId = FUNC_ID_BLINK_ORANGE_LED,
    .group      = BLIND_GROUP,
    .nextTask   = &BlinkOrangeLedTask,
};

struct Task1 configSettingsSoundTask = {
    .delay      = 3000,
    .functionId = FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND,
    .group      = BLIND_GROUP,
    .nextTask   = &configSettingsSoundTask,
};

const UserInterface blind1Interface = {
    .id                     = USER_INTERFACE_1_ID,
    .blinkLedTask           = &blinkRedLedTask,
    .ledId                  = LED_RED_ID,
    .configureSettingsSound = &configSettingsSoundTask,
};

const UserInterface blind2Interface = {
    .id                     = USER_INTERFACE_2_ID,
    .blinkLedTask           = &BlinkOrangeLedTask,
    .ledId                  = LED_ORANGE_ID,
    .configureSettingsSound = &configSettingsSoundTask,
};

UserInterface interfaces[NUM_INTERFACES] = {blind1Interface, blind2Interface};

#endif // USER_INTERFACE_CONFIG_H