/**
 * @file user_interface.h
 * @author Gian Barta-Dougall
 * @brief System file for user_interface
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

/* Public Includes */

/* Public STM Includes */
#include "stm32l4xx.h"
#include "task_scheduler_1.h"

/* Public #defines */
#define UI_ID_OFFSET        23
#define USER_INTERFACE_1_ID (0 + UI_ID_OFFSET)
#define USER_INTERFACE_2_ID (1 + UI_ID_OFFSET)

/* Public Structures and Enumerations */

typedef struct UserInterface {
    const uint8_t id;
    const uint8_t ledId;
    Task1* blinkLedTask;
    Task1* configureSettingsSound;
} UserInterface;

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void user_interface_init(void);

/**
 * @brief Turns on the the required LED in manual mode based
 * on the given id
 *
 * @param uiId The ID of the user interface to be turned on
 */
void user_interface_manual_mode_on(const uint8_t uiId);

/**
 * @brief Turns on the the required LED in daylight mode based
 * on the given id
 *
 * @param uiId The ID of the user interface to be turned on
 */
void user_interface_daylight_mode_on(const uint8_t uiId);

/**
 * @brief Turns on the the required LED in update settings mode
 * based on the given id
 *
 * @param uiId The ID of the user interface to be turned on
 */
void user_interface_update_settings_mode_on(const uint8_t uiId);

/**
 * @brief Turns the user interface of the given Id off
 *
 * @param uiId The id of the user interface to turn off
 */
void user_interface_off(const uint8_t uiId);

#endif // USER_INTERFACE_H
