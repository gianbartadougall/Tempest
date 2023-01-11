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
#include "utilities.h"

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

/**
 * @brief Returns the motor ID of the given blind
 *
 * @param blindId The ID of the given blind
 * @return uint8_t The motor ID of the given blind
 */
uint8_t blind_get_motor_id(uint8_t blindId);

/**
 * @brief Returns the encoder ID of the given blind
 *
 * @param blindId The ID of the given blind
 * @return uint8_t The encoder ID of the given blind
 */
uint8_t blind_get_encoder_id(uint8_t blindId);

/**
 * @brief Returns the alSensor ID of the given blind
 *
 * @param blindId The ID of the given blind
 * @return uint8_t The alSensor ID of the given blind
 */
uint8_t blind_get_al_sensor_id(uint8_t blindId);

/**
 * @brief Returns the current mode of the given blind
 *
 * @param blindId The ID of the given blind
 * @return uint8_t The current mode the given blind is in
 */
uint8_t blind_get_mode(uint8_t blindId);

/**
 * @brief Returns the ID of the currently selected blind
 *
 * @return uint8_t The ID of the currently selected blind
 */
uint8_t blind_get_selected_blind_id(void);

/**
 * @brief Sets the mode of the currently selected blind
 *
 * @param mode The mode to set the currently selected blind to
 */
void blind_set_selected_blind_mode(uint8_t mode);

/**
 * @brief Checks whether the encoder for each blind can be detected
 * or not. If it can the status of the blind is set to CONNECTED
 * else the status is set to DISCONNECTED
 */
void blind_update_connection_status(void);

/**
 * @brief Returns a pointer to the current selected blind
 *
 * @return Blind* A pointer to the current selected blind
 */
Blind* blind_get_blind_in_focus(void);

uint8_t blind_get_selected_blind_mode(void);
void blind_change_selected_blind(void);
void blind_play_error_sound(void);
void blind_revert_selected_blind_mode(void);

/**
 * @brief Calls private functions when required bit in flag is set
 */
void blind_process_internal_flags(void);

#endif // BLIND_H
