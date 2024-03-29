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

enum Mode { BLIND_MODE_MANUAL, BLIND_MODE_DAY_LIGHT, CONFIGURE_SETINGS };

#define NUM_BLINDS 2

#define BLIND_ID_OFFSET 52
#define BLIND_1_ID      (0 + BLIND_ID_OFFSET)
#define BLIND_2_ID      (1 + BLIND_ID_OFFSET)

enum BlindFunctions {
    FUNC_ID_BLINK_RED_LED,
    FUNC_ID_BLINK_ORANGE_LED,
    FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND,
    FUNC_ID_SWITCH_BLIND_MODE_TO_DAYLIGHT,
    FUNC_ID_BLINDS_DAY_LIGHT_UPDATE,
};

typedef struct Blind {
    const uint8_t id;
    const uint8_t blindMotorId;
    const uint8_t alSensorId;
    const uint8_t userInterfaceId;
    const Task1 switchToDayLightModeTask;
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
uint8_t blind_get_bif_id(void);

/**
 * @brief Sets the mode of the currently selected blind
 *
 * @param mode The mode to set the currently selected blind to
 */
void blind_set_bif_mode(uint8_t mode);

/**
 * @brief Returns a pointer to the current selected blind
 *
 * @return Blind* A pointer to the current selected blind
 */
Blind* blind_get_bif(void);

void blind_stop_bif_moving(void);
void blind_move_up(uint8_t blindId);
void blind_move_down(uint8_t blindId);
void blind_set_new_max_height(uint8_t blindId);
void blind_set_new_min_height(uint8_t blindId);
uint8_t blind_min_max_heights_are_valid(uint8_t blindId);

uint8_t blind_get_bif_mode(void);
void blind_toggle_bif(void);
void blind_play_error_sound(void);
void blind_revert_bif_mode(void);
void blind_process_internal_flags(void);
void blind_stop_moving(uint8_t blindId);

#endif // BLIND_H
