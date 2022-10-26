/**
 * @file tempest.c
 * @author Gian Barta-Dougall
 * @brief System file for tempest
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "tempest_config.h"
#include "hardware_config.h"
#include "button.h"
#include "blind.h"
#include "blind_motor.h"
#include "motor.h"

/* Private STM Includes */

/* Private #defines */

/* Private Structures and Enumerations */
extern uint32_t buttonTasksFlag;
extern uint32_t tempestTasksFlag;

/* Private Variable Declarations */
uint8_t blindIds[NUM_BLINDS] = {BLIND_1_ID, BLIND_2_ID};

/* Private Function Prototypes */
void tempest_process_external_button_flags(void);
void tempest_process_internal_flags(void);

/* Public Functions */

void tempest_init(void) {

    hardware_config_init();
    debug_clear();

    // Initialise all the required peripherals
    blind_motor_init(); // This needs to be first!
    ts_init();
    button_init();
    blind_init();

    debug_prints("Initialised 1\r\n");
}

/* Private Functions */

void tempest_update(void) {

    /****** START CODE BLOCK ******/
    // Description: Process all flags required. This ensures automatic
    // processes like button debouncing functions and that automatic
    // processes like reading the AL sensor and updating the task
    // scheduler occur

    button_process_internal_flags();
    ts_process_internal_flags();
    bm_process_internal_flags();
    al_sensor_process_internal_flags();
    tempest_process_external_button_flags();
    tempest_process_internal_flags();
    /****** END CODE BLOCK ******/

    // If any of the blinds are in automatic mode currently, update the
    // blind position if required
    for (uint8_t i = 0; i < NUM_BLINDS; i++) {

        if (blind_get_mode(blindIds[i]) != DAY_LIGHT) {
            continue;
        }

        uint8_t alSensorId = blind_get_al_sensor_id(blindIds[i]);
        uint8_t encoderId  = blind_get_encoder_id(blindIds[i]);
        uint8_t lightFound = al_sensor_light_found(alSensorId);

        if (lightFound == TRUE && encoder_at_min_height(encoderId)) {
            bm_move_blind_up(blindIds[i]);
        }

        if (lightFound == FALSE && encoder_at_max_height(encoderId)) {
            bm_move_blind_down(blindIds[i]);
        }
    }
}

void tempest_process_internal_flags(void) {

    if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_BLINK_RED_LED)) {
        FLAG_CLEAR(tempestTasksFlag, FUNC_ID_BLINK_RED_LED);
        led_toggle(LED_RED_ID);
    }

    if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_BLINK_ORANGE_LED)) {
        FLAG_CLEAR(tempestTasksFlag, FUNC_ID_BLINK_ORANGE_LED);
        led_toggle(LED_ORANGE_ID);
    }

    if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_SWITCH_BLIND_MODE_TO_DAYLIGHT)) {
        FLAG_CLEAR(tempestTasksFlag, FUNC_ID_SWITCH_BLIND_MODE_TO_DAYLIGHT);
        blind_set_selected_blind_mode(DAY_LIGHT);
    }

    if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND)) {
        FLAG_CLEAR(tempestTasksFlag, FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND);
        piezo_buzzer_play_sound(SOUND);
    }
}

void tempest_process_external_button_flags(void) {

    // Switch the current selected blind
    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD) &&
        FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD)) {

        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD);
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD);

        blind_change_selected_blind();
    }

    /****** START CODE BLOCK ******/
    // Description: Process single clicks from both button up and down

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_SINGLE_CLICK)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_SINGLE_CLICK);

        uint8_t blindId = blind_get_selected_blind_id();
        if (blind_get_selected_blind_mode() == MANUAL) {
            debug_prints("Moving blind up\r\n");
            bm_move_blind_up(blindId);
        }

        if (blind_get_selected_blind_mode() == CONFIGURE_SETINGS) {
            debug_prints("Setting max height\r\n");
            bm_set_new_max_height(blindId);
        }
    }

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_SINGLE_CLICK)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_SINGLE_CLICK);

        uint8_t blindId = blind_get_selected_blind_id();

        if (blind_get_selected_blind_mode() == MANUAL) {
            debug_prints("Moving blind down\r\n");
            bm_move_blind_down(blindId);
        }

        if (blind_get_selected_blind_mode() == CONFIGURE_SETINGS) {
            debug_prints("Setting min height\r\n");
            bm_set_new_min_height(blindId);
        }
    }

    /****** END CODE BLOCK ******/

    /****** START CODE BLOCK ******/
    // Description: Process double clicks from both button up and button down

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_DOUBLE_CLICK)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_DOUBLE_CLICK);

        uint8_t blindId = blind_get_selected_blind_id();
        bm_stop_blind_moving(blindId);

        switch (blind_get_selected_blind_mode()) {
            case MANUAL:
            case DAY_LIGHT:
                blind_set_selected_blind_mode(CONFIGURE_SETINGS);
                break;
            case CONFIGURE_SETINGS:

                if (bm_min_max_heights_are_valid(blindId) == TRUE) {
                    blind_revert_selected_blind_mode();
                } else {
                    blind_play_error_sound();
                }

                break;
            default:
                break;
        }
    }

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_DOUBLE_CLICK)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_DOUBLE_CLICK);

        uint8_t blindId = blind_get_selected_blind_id();
        bm_stop_blind_moving(blindId);

        if (blind_get_selected_blind_mode() == MANUAL) {
            blind_set_selected_blind_mode(DAY_LIGHT);
        }

        if (blind_get_selected_blind_mode() == DAY_LIGHT) {
            debug_prints("Changed to manual mode\r\n");
            blind_set_selected_blind_mode(MANUAL);
        }
    }

    // /****** END CODE BLOCK ******/

    // /****** START CODE BLOCK ******/
    // // Description: Process press and holds from both button up and button down

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD);

        if ((blind_get_selected_blind_mode() == MANUAL) || (blind_get_selected_blind_mode() == CONFIGURE_SETINGS)) {
            bm_move_blind_up(blind_get_selected_blind_id());
            debug_prints("Move blind up\r\n");
        }
    }

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD);

        if ((blind_get_selected_blind_mode() == MANUAL) || (blind_get_selected_blind_mode() == CONFIGURE_SETINGS)) {
            bm_move_blind_down(blind_get_selected_blind_id());
            debug_prints("Move blind down\r\n");
        }
    }

    // /****** END CODE BLOCK ******/

    // /****** START CODE BLOCK ******/
    // // Description: Process the release of a press and hold from both button up and button down

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD_RELEASED)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD_RELEASED);

        if ((blind_get_selected_blind_mode() == MANUAL) || (blind_get_selected_blind_mode() == CONFIGURE_SETINGS)) {
            bm_stop_blind_moving(blind_get_selected_blind_id());
            debug_prints("Stop moving blind\r\n");
        }
    }

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD_RELEASED)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD_RELEASED);

        if ((blind_get_selected_blind_mode() == MANUAL) || (blind_get_selected_blind_mode() == CONFIGURE_SETINGS)) {
            bm_stop_blind_moving(blind_get_selected_blind_id());
            debug_prints("Stop moving blind\r\n");
        }
    }

    /****** END CODE BLOCK ******/
}