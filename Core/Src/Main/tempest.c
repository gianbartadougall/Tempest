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

/* Private STM Includes */

/* Private #defines */

/* Private Structures and Enumerations */

Blind* selectedBlind         = NULL;
uint8_t selectedBlindUpdated = FALSE;

extern uint32_t buttonTasksFlag;
extern uint32_t encoderTaskFlags;
extern uint32_t tempestTasksFlag;

/* Private Function Prototypes */

/* Public Functions */

void tempest_init(void) {

    hardware_config_init();
    debug_clear();

#ifdef ENCODER_MODULE_ENABLED
    encoder_init();
#endif

    // Initialise all the required peripherals
    ts_init();
    button_init();
}

/* Private Functions */
void tempest_update_user_signals(void);
void tempest_process_peripheral_flags(void);
void tempest_check_for_connected_blinds(void);
void tempest_update_external_encoder_flags(void);
void tempest_update_external_button_flags(void);
void tempest_move_blind_up(Blind* blind);
void tempest_move_blind_down(Blind* blind);
void tempest_stop_blind_moving(Blind* blind);
void tempest_update_flags(void);
void tempest_turn_on_signals(Blind* blind);
void tempest_turn_off_signals(Blind* blind);
void tempest_set_new_mode(Blind* blind, uint8_t mode);

void tempest_update(void) {

    // Check which blinds are currently connected to the system
    tempest_check_for_connected_blinds();

    // Ensure at least one blind is connected the system
    // if (selectedBlind == NULL) {
    //     debug_prints("Not detected\r\n");
    //     HAL_Delay(1000);
    //     return;
    // } else {
    //     debug_prints("Detected\r\n");
    //     HAL_Delay(1000);
    // }

    // Process flags in peripheral driver files like the Ambient Light Sensor and Button
    // This ensures that automatic process like debouncing run correctly
    tempest_process_peripheral_flags();

    // Process external flags to be handled by the main loop. These flags dictate logic
    // of the system that needs to occur such as moving a motor up or down or changing
    // the current mode of one of the blinds
    tempest_update_external_button_flags();
    tempest_update_flags();
    // tempest_update_external_encoder_flags();

    // for (uint8_t i = 0; i < MAX_NUM_BLINDS; i++) {
    //     uint8_t lightFound = al_sensor_light_found(blinds[i]->alSensorId);
    //     if (lightFound == TRUE) {
    //         tempest_move_blind_up(blinds[i]);
    //     }

    //     if (lightFound == FALSE) {
    //         tempest_move_blind_down(blinds[i]);
    //     }
    // }
}

void tempest_update_flags(void) {

    if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_BLINK_RED_LED)) {
        FLAG_CLEAR(tempestTasksFlag, FUNC_ID_BLINK_RED_LED);
        led_toggle(LED_RED_ID);
    }

    if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_BLINK_ORANGE_LED)) {
        FLAG_CLEAR(tempestTasksFlag, FUNC_ID_BLINK_ORANGE_LED);
        led_toggle(LED_ORANGE_ID);
    }

    if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_SWITCH_BLIND_1_MODE_TO_DAYLIGHT)) {
        FLAG_CLEAR(tempestTasksFlag, FUNC_ID_SWITCH_BLIND_1_MODE_TO_DAYLIGHT);
        tempest_set_new_mode(&Blind1, DAY_LIGHT);
    }

    if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_SWITCH_BLIND_2_MODE_TO_DAYLIGHT)) {
        FLAG_CLEAR(tempestTasksFlag, FUNC_ID_SWITCH_BLIND_2_MODE_TO_DAYLIGHT);
        tempest_set_new_mode(&Blind2, DAY_LIGHT);
    }

    if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND)) {
        FLAG_CLEAR(tempestTasksFlag, FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND);
        piezo_buzzer_play_sound(SOUND);
    }

    if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_PRINT_TIMER_COUNT)) {
        FLAG_CLEAR(tempestTasksFlag, FUNC_ID_PRINT_TIMER_COUNT);
        char m[60];
        sprintf(m, "TIM: %li\r\n", TIM1->CNT);
        debug_prints(m);
    }
}

void tempest_turn_off_signals(Blind* blind) {
    // Turn off all signals for the unselected blind
    debug_prints("Cancelling tasks\r\n");
    ts_cancel_running_task(blind->blinkLedTask);
    ts_cancel_running_task(blind->configSettingsSoundTask);
    led_off(blind->ledId);
}

void tempest_turn_on_signals(Blind* blind) {

    // Start the appropriate signal for the selected blind
    if (blind->mode == MANUAL) {
        ts_add_task_to_queue(blind->blinkLedTask);
    }

    if (blind->mode == DAY_LIGHT) {
        led_on(blind->ledId);
    }

    if (blind->mode == CONFIGURE_SETINGS) {
        ts_add_task_to_queue(blind->blinkLedTask);
        ts_add_task_to_queue(blind->configSettingsSoundTask);
    }
}

void tempest_process_peripheral_flags(void) {
    button_process_internal_flags();
    ts_process_internal_flags();
    // al_sensor_process_internal_flags();
}

void tempest_check_for_connected_blinds(void) {

    // Assume no blinds are connected as this is safer then assuming blinds are conncted
    // Blind1.status = encoder_probe_connection(Blind1.encoderId);
    // Blind2.status = encoder_probe_connection(Blind2.encoderId);
    Blind1.status = CONNECTED;
    Blind2.status = CONNECTED;

    /* Update the selected blind if required */
    if ((Blind1.status == CONNECTED && selectedBlind == &Blind1) ||
        (Blind2.status == CONNECTED && selectedBlind == &Blind2)) {
        return;
    }

    if (Blind1.status == CONNECTED && (selectedBlind == NULL || Blind2.status == DISCONNECTED)) {
        selectedBlind = &Blind1;
        tempest_turn_on_signals(&Blind1);
        return;
    }

    if (Blind2.status == CONNECTED && (selectedBlind == NULL || Blind1.status == DISCONNECTED)) {
        selectedBlind = &Blind2;
        tempest_turn_on_signals(&Blind2);
        return;
    }

    // debug_prints("MSG: No blind connected\r\n");
    selectedBlind = NULL;
}

void tempest_update_external_encoder_flags(void) {

    if (FLAG_IS_SET(encoderTaskFlags, ENCODER_1_LIMIT_REACHED)) {
        FLAG_CLEAR(encoderTaskFlags, ENCODER_1_LIMIT_REACHED);
        tempest_stop_blind_moving(&Blind1);
    }

    if (FLAG_IS_SET(encoderTaskFlags, ENCODER_2_LIMIT_REACHED)) {
        FLAG_CLEAR(encoderTaskFlags, ENCODER_2_LIMIT_REACHED);
        tempest_stop_blind_moving(&Blind2);
    }
}

void tempest_update_external_button_flags(void) {

    // Switch which blind is currently selected if both buttons are pressed
    if ((button_get_state(BUTTON_UP_ID) == BUTTON_PRESSED) && (button_get_state(BUTTON_DOWN_ID) == BUTTON_PRESSED)) {

        // Switch the current selected blind
        if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD) &&
            FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD)) {

            FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD);

            if (selectedBlind == &Blind1 && Blind2.status == CONNECTED) {
                selectedBlind = &Blind2;
                tempest_turn_off_signals(&Blind1);
                tempest_turn_on_signals(&Blind2);
                debug_prints("Selected Blind 2\r\n");
            };

            if (selectedBlind == &Blind2 && Blind1.status == CONNECTED) {
                selectedBlind = &Blind1;
                tempest_turn_off_signals(&Blind2);
                tempest_turn_on_signals(&Blind1);
                debug_prints("Selected Blind 1\r\n");
            };
        }

        return;
    }

    /****** START CODE BLOCK ******/
    // Description: Process single clicks from both button up and down

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_SINGLE_CLICK)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_SINGLE_CLICK);

        if (selectedBlind->mode == MANUAL) {
            debug_prints("Moving blind up\r\n");
            tempest_move_blind_up(selectedBlind);
        }

        if (selectedBlind->mode == CONFIGURE_SETINGS) {
            debug_prints("Setting max height\r\n");
            encoder_set_max_height(selectedBlind->encoderId);
        }
    }

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_SINGLE_CLICK)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_SINGLE_CLICK);

        if (selectedBlind->mode == MANUAL) {
            debug_prints("Moving blind down\r\n");
            tempest_move_blind_down(selectedBlind);
        }

        if (selectedBlind->mode == CONFIGURE_SETINGS) {
            debug_prints("Setting min height\r\n");
            encoder_set_min_height(selectedBlind->encoderId);
        }
    }

    /****** END CODE BLOCK ******/

    /****** START CODE BLOCK ******/
    // Description: Process double clicks from both button up and button down

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_DOUBLE_CLICK)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_DOUBLE_CLICK);

        tempest_stop_blind_moving(selectedBlind);

        switch (selectedBlind->mode) {
            case MANUAL:
            case DAY_LIGHT:
                encoder_disable_limits(selectedBlind->encoderId);
                debug_prints("Disable limits\r\n");
                tempest_set_new_mode(selectedBlind, CONFIGURE_SETINGS);
                break;
            case CONFIGURE_SETINGS:

                ts_cancel_running_task(selectedBlind->configSettingsSoundTask);

                if (encoder_limits_are_valid(selectedBlind->encoderId) == TRUE) {
                    tempest_set_new_mode(selectedBlind, selectedBlind->previousMode);
                    debug_prints("Back to previous mode\r\n");
                } else {
                    piezo_buzzer_play_sound(PIEZO_ERROR_SOUND);
                    HAL_Delay(100);
                    piezo_buzzer_play_sound(PIEZO_ERROR_SOUND);
                    ts_add_task_to_queue(selectedBlind->configSettingsSoundTask);
                }
                break;
            default:
                break;
        }
    }

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_DOUBLE_CLICK)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_DOUBLE_CLICK);

        tempest_stop_blind_moving(selectedBlind);

        if (selectedBlind->mode == MANUAL) {
            // Cancel switch to day light mode if manual button is pressed again quick enough
            if (ts_cancel_running_task(&selectedBlind->switchToDayLightModeTask) == TS_TASK_NOT_FOUND) {
                debug_prints("Changing to auto mode in 5 seconds\r\n");
                ts_add_task_to_queue(&selectedBlind->switchToDayLightModeTask);
            }
        }

        if (selectedBlind->mode == DAY_LIGHT) {
            debug_prints("Changed to manual mode\r\n");
            tempest_set_new_mode(selectedBlind, MANUAL);
        }
    }

    /****** END CODE BLOCK ******/

    /****** START CODE BLOCK ******/
    // Description: Process press and holds from both button up and button down

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD);

        if ((selectedBlind->mode == MANUAL) || (selectedBlind->mode == CONFIGURE_SETINGS)) {
            tempest_move_blind_up(selectedBlind);
            debug_prints("Move blind up\r\n");
        }
    }

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD);

        if ((selectedBlind->mode == MANUAL) || (selectedBlind->mode == CONFIGURE_SETINGS)) {
            tempest_move_blind_down(selectedBlind);
            debug_prints("Move blind down\r\n");
        }
    }

    /****** END CODE BLOCK ******/

    /****** START CODE BLOCK ******/
    // Description: Process the release of a press and hold from both button up and button down

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD_RELEASED)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD_RELEASED);

        if ((selectedBlind->mode == MANUAL) || (selectedBlind->mode == CONFIGURE_SETINGS)) {
            tempest_stop_blind_moving(selectedBlind);
            debug_prints("Stop moving blind\r\n");
        }
    }

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD_RELEASED)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD_RELEASED);

        if ((selectedBlind->mode == MANUAL) || (selectedBlind->mode == CONFIGURE_SETINGS)) {
            tempest_stop_blind_moving(selectedBlind);
            debug_prints("Stop moving blind\r\n");
        }
    }

    /****** END CODE BLOCK ******/
}

void tempest_set_new_mode(Blind* blind, uint8_t mode) {

    tempest_turn_off_signals(blind);

    // Set new mode before turning signals back on
    blind->mode = mode;

    tempest_turn_on_signals(blind);
}

void tempest_move_blind_up(Blind* blind) {

    if (blind->mode != CONFIGURE_SETINGS) {
        if (encoder_at_max_height(blind->encoderId) == TRUE) {
            return;
        }
    }

    ts_add_task_to_queue(&printTimerCount);
    // Update the direction and move blind up
    encoder_set_direction_up(blind->encoderId);
    motor_forward(blind->motorId);
}

void tempest_move_blind_down(Blind* blind) {

    if (blind->mode != CONFIGURE_SETINGS) {
        if (encoder_at_min_height(blind->encoderId) == TRUE) {
            debug_prints("return\r\n");
            return;
        }
    }

    debug_prints("Direction down\r\n");
    ts_add_task_to_queue(&printTimerCount);
    // Update the direction and move blind up
    encoder_set_direction_down(blind->encoderId);
    motor_reverse(blind->motorId);
}

void tempest_stop_blind_moving(Blind* blind) {
    motor_brake(blind->motorId);
    ts_cancel_running_task(&printTimerCount);
}