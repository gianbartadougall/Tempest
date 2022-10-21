/**
 * @file blind.h
 * @author Gian Barta-Dougall
 * @brief System file for blind
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef BLIND_H
#define BLIND_H

#include "stm32l4xx_hal.h"
#include "task_scheduler_1.h"

enum Mode { MANUAL, DAY_LIGHT, CONFIGURE_SETINGS };

#define NUM_BLINDS 2

#define BLIND_ID_OFFSET 52
#define BLIND_1_ID      (0 + BLIND_ID_OFFSET)
#define BLIND_2_ID      (1 + BLIND_ID_OFFSET)

enum TempestFunctions {
    FUNC_ID_SWITCH_BLIND_MODE_TO_DAYLIGHT,
    FUNC_ID_BLINK_RED_LED,
    FUNC_ID_BLINK_ORANGE_LED,
    FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND,
};

typedef struct Blind {
    const uint8_t id;
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

/* Public Variable Declarations */

/**
 * @brief Initialise the system library.
 */
void blind_init(void);

uint8_t blind_get_motor_id(uint8_t blindId);
uint8_t blind_get_encoder_id(uint8_t blindId);
uint8_t blind_get_al_sensor_id(uint8_t blindId);
uint8_t blind_get_mode(uint8_t blindId);
uint8_t blind_get_selected_blind_id(void);
void blind_set_selected_blind_mode(uint8_t mode);
void blind_update_connection_status(void);
Blind* blind_get_blind_in_focus(void);
uint8_t blind_get_selected_blind_mode(void);
void blind_change_selected_blind(void);
void blind_play_error_sound(void);
void blind_revert_selected_blind_mode(void);

#endif // BLIND_H
