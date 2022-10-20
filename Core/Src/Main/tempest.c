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

/* Private STM Includes */

/* Private #defines */

/* Private Structures and Enumerations */
extern uint32_t buttonTasksFlag;
extern uint32_t encoderTaskFlags;
extern uint32_t tempestTasksFlag;

/* Private Function Prototypes */
void tempest_update_user_signals(void);
void tempest_process_peripheral_flags(void);
void tempest_confirm_encoders_connected(void);
void tempest_update_external_encoder_flags(void);
void tempest_update_external_button_flags(void);
void tempest_move_blind_up(Blind* blind);
void tempest_move_blind_down(Blind* blind);
void tempest_stop_blind_moving(Blind* blind);
void tempest_update_flags(void);
void tempest_turn_on_signals(Blind* blind);
void tempest_turn_off_signals(Blind* blind);
void tempest_set_new_mode(Blind* blind, uint8_t mode);
void tempest_align_encoder_position(Blind* blind);
void tempest_update_selected_blind(void);

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
    blind_motor_init();
    // ts_add_task_to_queue(&printTimerCount);
}

/* Private Functions */

void tempest_update(void) {

    // Confirm the encoders on the blinds are connected
    // tempest_confirm_encoders_connected();
    // tempest_process_peripheral_flags();

    // Switch which blind is currently selected if both buttons are pressed
    if ((button_get_state(BUTTON_UP_ID) == BUTTON_PRESSED) && (button_get_state(BUTTON_DOWN_ID) == BUTTON_PRESSED)) {
        blind_update_connection_status();
        return;
    }

    // Ensure at least one blind is connected the system
    if (blind_get_blind_in_focus()->status == DISCONNECTED) {
        tempestTasksFlag = 0;
        encoderTaskFlags = 0;
        buttonTasksFlag  = 0;
        HAL_Delay(1000);
        debug_prints("No blinds alligned\r\n");
        return;
    }

    char m[60];
    sprintf(m, "Blind in focus: %i\r\n", blind_get_blind_in_focus()->id);
    debug_prints(m);

    HAL_Delay(1000);

    // Process flags in peripheral driver files like the Ambient Light Sensor and Button
    // This ensures that automatic process like debouncing run correctly

    // Process external flags to be handled by the main loop. These flags dictate logic
    // of the system that needs to occur such as moving a motor up or down or changing
    // the current mode of one of the blinds
    // tempest_update_external_button_flags();
    // tempest_update_flags();
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

// void tempest_update_flags(void) {

//     if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_BLINK_RED_LED)) {
//         FLAG_CLEAR(tempestTasksFlag, FUNC_ID_BLINK_RED_LED);
//         led_toggle(LED_RED_ID);
//     }

//     if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_BLINK_ORANGE_LED)) {
//         FLAG_CLEAR(tempestTasksFlag, FUNC_ID_BLINK_ORANGE_LED);
//         led_toggle(LED_ORANGE_ID);
//     }

//     if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_SWITCH_BLIND_1_MODE_TO_DAYLIGHT)) {
//         FLAG_CLEAR(tempestTasksFlag, FUNC_ID_SWITCH_BLIND_1_MODE_TO_DAYLIGHT);
//         tempest_set_new_mode(&Blind1, DAY_LIGHT);
//     }

//     if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_SWITCH_BLIND_2_MODE_TO_DAYLIGHT)) {
//         FLAG_CLEAR(tempestTasksFlag, FUNC_ID_SWITCH_BLIND_2_MODE_TO_DAYLIGHT);
//         tempest_set_new_mode(&Blind2, DAY_LIGHT);
//     }

//     if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND)) {
//         FLAG_CLEAR(tempestTasksFlag, FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND);
//         piezo_buzzer_play_sound(SOUND);
//     }

//     if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_CONFIRM_ENCODER_1_IN_OPERATION)) {
//         FLAG_CLEAR(tempestTasksFlag, FUNC_ID_CONFIRM_ENCODER_1_IN_OPERATION);
//         if (encoder1LastCount == encoder_get_count(Blind1.encoderId)) {
//             tempest_stop_blind_1_moving();
//         }
//     }

//     if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_CONFIRM_ENCODER_2_IN_OPERATION)) {
//         FLAG_CLEAR(tempestTasksFlag, FUNC_ID_CONFIRM_ENCODER_2_IN_OPERATION);
//         if (encoder2LastCount == encoder_get_count(Blind2.encoderId)) {
//             tempest_stop_blind_2_moving();
//         }
//     }

//     if (FLAG_IS_SET(tempestTasksFlag, FUNC_ID_PRINT_TIMER_COUNT)) {
//         FLAG_CLEAR(tempestTasksFlag, FUNC_ID_PRINT_TIMER_COUNT);
//         char m[60];
//         sprintf(m, "TIM: %li\r\n", TIM1->CNT);
//         debug_prints(m);
//     }
// }

// void tempest_turn_off_signals(Blind* blind) {
//     // Turn off all signals for the unselected blind
//     debug_prints("Cancelling tasks\r\n");
//     ts_cancel_running_task(blind->blinkLedTask);
//     ts_cancel_running_task(blind->configSettingsSoundTask);
//     led_off(blind->ledId);
// }

// void tempest_turn_on_signals(Blind* blind) {

//     // Start the appropriate signal for the selected blind
//     if (blind->mode == MANUAL) {
//         ts_add_task_to_queue(blind->blinkLedTask);
//     }

//     if (blind->mode == DAY_LIGHT) {
//         led_on(blind->ledId);
//     }

//     if (blind->mode == CONFIGURE_SETINGS) {
//         ts_add_task_to_queue(blind->blinkLedTask);
//         ts_add_task_to_queue(blind->configSettingsSoundTask);
//     }
// }

// void tempest_process_peripheral_flags(void) {
//     button_process_internal_flags();
//     ts_process_internal_flags();
//     // al_sensor_process_internal_flags();
// }

// void tempest_confirm_encoders_connected(void) {

//     // Only check connection if motor for blind is not on
//     for (uint8_t i = 0; i < MAX_NUM_BLINDS; i++) {

//         // Once the blind has been disconnected, don't keep checking
//         if (blinds[i]->encoderAligned == FALSE) {
//             continue;
//         }

//         uint8_t motorState = motor_get_state(blinds[i]->motorId);
//         if (motorState == MOTOR_FORWARD || motorState == MOTOR_REVERSE) {
//             continue;
//         }

//         // Motor is currently still, probe for encoder connection
//         if (encoder_get_state(blinds[i]->encoderId) == PIN_HIGH) {
//             continue;
//         }

//         // Try align encoder
//         tempest_align_encoder_position(blinds[i]);
//     }

//     if ((selectedBlind == &Blind1) && (Blind1.encoderAligned == FALSE) && (Blind2.encoderAligned == TRUE)) {
//         selectedBlind = &Blind2;
//     }

//     if ((selectedBlind == &Blind2) && (Blind2.encoderAligned == FALSE) && (Blind1.encoderAligned == TRUE)) {
//         selectedBlind = &Blind1;
//     }
// }

// void tempest_update_external_encoder_flags(void) {

//     if (FLAG_IS_SET(encoderTaskFlags, ENCODER_1_LIMIT_REACHED)) {
//         FLAG_CLEAR(encoderTaskFlags, ENCODER_1_LIMIT_REACHED);
//         tempest_stop_blind_moving(&Blind1);
//     }

//     if (FLAG_IS_SET(encoderTaskFlags, ENCODER_2_LIMIT_REACHED)) {
//         FLAG_CLEAR(encoderTaskFlags, ENCODER_2_LIMIT_REACHED);
//         tempest_stop_blind_moving(&Blind2);
//     }
// }

// void tempest_update_selected_blind(void) {

//     // Switch the current selected blind
//     if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD) &&
//         FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD)) {

//         FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD);
//         FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD);

//         // Try align both blinds
//         debug_prints("Aligning both blinds\r\n");
//         tempest_align_encoder_position(&Blind1);
//         tempest_align_encoder_position(&Blind2);

//         if (selectedBlind == &Blind1 && Blind2.encoderAligned == FALSE) {

//             if (Blind2.encoderAligned == TRUE) {
//                 debug_prints("Selected Blind 2\r\n");
//                 selectedBlind = &Blind2;
//                 tempest_turn_off_signals(&Blind1);
//                 tempest_turn_on_signals(&Blind2);
//             }
//         } else {

//             if (Blind1.encoderAligned == TRUE) {
//                 debug_prints("Selected Blind 1\r\n");
//                 selectedBlind = &Blind1;
//                 tempest_turn_off_signals(&Blind2);
//                 tempest_turn_on_signals(&Blind1);
//             }
//         }
//     }
// }

// void tempest_update_external_button_flags(void) {

//     /****** START CODE BLOCK ******/
//     // Description: Process single clicks from both button up and down

//     if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_SINGLE_CLICK)) {
//         FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_SINGLE_CLICK);

//         if (selectedBlind->mode == MANUAL) {
//             debug_prints("Moving blind up\r\n");
//             tempest_move_blind_up(selectedBlind);
//         }

//         if (selectedBlind->mode == CONFIGURE_SETINGS) {
//             debug_prints("Setting max height\r\n");
//             encoder_set_max_height(selectedBlind->encoderId);
//         }
//     }

//     if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_SINGLE_CLICK)) {
//         FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_SINGLE_CLICK);

//         if (selectedBlind->mode == MANUAL) {
//             debug_prints("Moving blind down\r\n");
//             tempest_move_blind_down(selectedBlind);
//         }

//         if (selectedBlind->mode == CONFIGURE_SETINGS) {
//             debug_prints("Setting min height\r\n");
//             encoder_set_min_height(selectedBlind->encoderId);
//         }
//     }

//     /****** END CODE BLOCK ******/

//     /****** START CODE BLOCK ******/
//     // Description: Process double clicks from both button up and button down

//     if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_DOUBLE_CLICK)) {
//         FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_DOUBLE_CLICK);

//         tempest_stop_blind_moving(selectedBlind);

//         switch (selectedBlind->mode) {
//             case MANUAL:
//             case DAY_LIGHT:
//                 encoder_disable_limits(selectedBlind->encoderId);
//                 debug_prints("Disable limits\r\n");
//                 tempest_set_new_mode(selectedBlind, CONFIGURE_SETINGS);
//                 break;
//             case CONFIGURE_SETINGS:

//                 ts_cancel_running_task(selectedBlind->configSettingsSoundTask);

//                 if (encoder_limits_are_valid(selectedBlind->encoderId) == TRUE) {
//                     tempest_set_new_mode(selectedBlind, selectedBlind->previousMode);
//                     debug_prints("Back to previous mode\r\n");
//                 } else {
//                     piezo_buzzer_play_sound(PIEZO_ERROR_SOUND);
//                     HAL_Delay(100);
//                     piezo_buzzer_play_sound(PIEZO_ERROR_SOUND);
//                     ts_add_task_to_queue(selectedBlind->configSettingsSoundTask);
//                 }
//                 break;
//             default:
//                 break;
//         }
//     }

//     if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_DOUBLE_CLICK)) {
//         FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_DOUBLE_CLICK);

//         tempest_stop_blind_moving(selectedBlind);

//         if (selectedBlind->mode == MANUAL) {
//             // Cancel switch to day light mode if manual button is pressed again quick enough
//             if (ts_cancel_running_task(&selectedBlind->switchToDayLightModeTask) == TS_TASK_NOT_FOUND) {
//                 debug_prints("Changing to auto mode in 5 seconds\r\n");
//                 ts_add_task_to_queue(&selectedBlind->switchToDayLightModeTask);
//             }
//         }

//         if (selectedBlind->mode == DAY_LIGHT) {
//             debug_prints("Changed to manual mode\r\n");
//             tempest_set_new_mode(selectedBlind, MANUAL);
//         }
//     }

//     /****** END CODE BLOCK ******/

//     /****** START CODE BLOCK ******/
//     // Description: Process press and holds from both button up and button down

//     if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD)) {
//         FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD);

//         if ((selectedBlind->mode == MANUAL) || (selectedBlind->mode == CONFIGURE_SETINGS)) {
//             tempest_move_blind_up(selectedBlind);
//             debug_prints("Move blind up\r\n");
//         }
//     }

//     if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD)) {
//         FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD);

//         if ((selectedBlind->mode == MANUAL) || (selectedBlind->mode == CONFIGURE_SETINGS)) {
//             tempest_move_blind_down(selectedBlind);
//             debug_prints("Move blind down\r\n");
//         }
//     }

//     /****** END CODE BLOCK ******/

//     /****** START CODE BLOCK ******/
//     // Description: Process the release of a press and hold from both button up and button down

//     if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD_RELEASED)) {
//         FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD_RELEASED);

//         if ((selectedBlind->mode == MANUAL) || (selectedBlind->mode == CONFIGURE_SETINGS)) {
//             tempest_stop_blind_moving(selectedBlind);
//             debug_prints("Stop moving blind\r\n");
//         }
//     }

//     if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD_RELEASED)) {
//         FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD_RELEASED);

//         if ((selectedBlind->mode == MANUAL) || (selectedBlind->mode == CONFIGURE_SETINGS)) {
//             tempest_stop_blind_moving(selectedBlind);
//             debug_prints("Stop moving blind\r\n");
//         }
//     }

//     /****** END CODE BLOCK ******/
// }

// void tempest_set_new_mode(Blind* blind, uint8_t mode) {

//     tempest_turn_off_signals(blind);

//     // Set new mode before turning signals back on
//     blind->mode = mode;

//     tempest_turn_on_signals(blind);
// }

// void tempest_move_blind_up(Blind* blind) {

//     if (blind->mode != CONFIGURE_SETINGS) {
//         if (encoder_at_max_height(blind->encoderId) == TRUE) {
//             return;
//         }
//     }

//     // Update the direction and move blind up
//     encoder_set_direction_up(blind->encoderId);
//     ts_add_task_to_queue(&printTimerCount);
//     blind->lastEncoderCount = encoder_get_count(blind->encoderId);
//     ts_add_task_to_queue(blind->confirmEncoderInOperation);
//     motor_forward(blind->motorId);
// }

// void tempest_move_blind_down(Blind* blind) {

//     if (blind->mode != CONFIGURE_SETINGS) {
//         if (encoder_at_min_height(blind->encoderId) == TRUE) {
//             debug_prints("return\r\n");
//             return;
//         }
//     }

//     // Update the direction and move blind up
//     debug_prints("Move blind down\r\n");
//     ts_add_task_to_queue(&printTimerCount);
//     encoder_set_direction_down(blind->encoderId);
//     blind->lastEncoderCount = encoder_get_count(blind->encoderId);
//     ts_add_task_to_queue(blind->confirmEncoderInOperation);
//     motor_reverse(blind->motorId);
// }

// void tempest_stop_blind_moving(Blind* blind) {
//     motor_brake(blind->motorId);
//     ts_cancel_running_task(&printTimerCount);
//     ts_cancel_running_task(blind->confirmEncoderInOperation);
// }

// void tempest_stop_blind_1_moving(void) {
//     ts_cancel_running_task(&printTimerCount);
//     tempest_stop_blind_moving(&Blind1);
// }

// void tempest_stop_blind_2_moving(void) {
//     ts_cancel_running_task(&printTimerCount);
//     tempest_stop_blind_moving(&Blind2);
// }

// void tempest_align_encoder_position(Blind* blind) {

// // Set timeout. This ensures the motor will automatically
// // stop if the encoder has not aligned within the required
// // time period
// uint32_t timeout = (HAL_GetTick() + 500) % __32_BIT_MAX_COUNT;

// motor_reverse(blind->motorId);

// // char m[60];
// while ((encoder_get_state(blind->encoderId) != PIN_HIGH) && (HAL_GetTick() < timeout)) {
//     // sprintf(m, "%li\r\n", HAL_GetTick());
//     // debug_prints(m);
// }

// tempest_stop_blind_moving(blind);
// char m[60];
// sprintf(m, "Encoder  IDR: %i\r\n", encoder_get_state(blind->encoderId));
// debug_prints(m);

// if (encoder_get_state(blind->encoderId) != PIN_HIGH) {
//     blind->encoderAligned = FALSE;
// } else {
//     blind->encoderAligned = TRUE;
// }
// }