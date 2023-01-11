/**
 * @file user_interface.c
 * @author Gian Barta-Dougall
 * @brief System file for user_interface
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "user_interface_config.h"
#include "task_scheduler_1.h"

/* Private STM Includes */

/* Private #defines */
#define ASSERT_VALID_UI_ID(id)                                                          \
    do {                                                                                \
        if ((id < UI_ID_OFFSET) || (id > (NUM_INTERFACES - 1 + UI_ID_OFFSET))) {        \
            char msg[100];                                                              \
            sprintf(msg, "Invalid ID: File %s line number %d\r\n", __FILE__, __LINE__); \
            log_prints(msg);                                                            \
            return;                                                                     \
        }                                                                               \
    } while (0)

#define UI_ID_TO_INDEX(id) (id - UI_ID_OFFSET)

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */

/* Public Functions */

void user_interface_manual_mode_on(const uint8_t uiId) {

    ASSERT_VALID_UI_ID(uiId);

    const uint8_t index = UI_ID_TO_INDEX(uiId);
    ts_add_task_to_queue(interfaces[index].blinkLedTask);
}

void user_interface_daylight_mode_on(const uint8_t uiId) {

    ASSERT_VALID_UI_ID(uiId);

    const uint8_t index = UI_ID_TO_INDEX(uiId);
    led_on(interfaces[index].ledId);
}

void user_interface_update_settings_mode_on(const uint8_t uiId) {

    ASSERT_VALID_UI_ID(uiId);

    const uint8_t index = UI_ID_TO_INDEX(uiId);
    ts_add_task_to_queue(interfaces[index].blinkLedTask);
    ts_add_task_to_queue(interfaces[index].configureSettingsSound);
}

void user_interface_off(const uint8_t uiId) {
    ASSERT_VALID_UI_ID(uiId);

    const uint8_t index = UI_ID_TO_INDEX(uiId);
    ts_cancel_running_task(interfaces[index].blinkLedTask);
    ts_cancel_running_task(interfaces[index].configureSettingsSound);
    led_off(interfaces[index].ledId);
}

/* Private Functions */