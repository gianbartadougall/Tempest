/**
 * @file blind.c
 * @author Gian Barta-Dougall
 * @brief System file for blind
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "blind.h"
#include "led.h"
#include "blind_motor.h"
#include "motor.h"
#include "ambient_light_sensor.h"
#include "encoder.h"
#include "utilities.h"
#include "piezo_buzzer.h"

/* Private STM Includes */

/* Private #defines */
#define BLIND_ID_INVALID(id)  ((id < BLIND_ID_OFFSET) || (id > (NUM_BLINDS - 1 + BLIND_ID_OFFSET)))
#define BLIND_ID_TO_INDEX(id) (id - BLIND_ID_OFFSET)

/* Private Structures and Enumerations */

const struct Task1 switchBlindToDayLightModeTask = {
    .delay      = 5000,
    .functionId = FUNC_ID_SWITCH_BLIND_MODE_TO_DAYLIGHT,
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

Blind Blind1 = {
    .id                       = BLIND_1_ID,
    .motorId                  = MOTOR_1_ID,
    .encoderId                = ENCODER_1_ID,
    .alSensorId               = AL_SENSOR_1_ID,
    .ledId                    = LED_RED_ID,
    .switchToDayLightModeTask = switchBlindToDayLightModeTask,
    .blinkLedTask             = &blinkRedLedTask,
    .configSettingsSoundTask  = &configSettingsSoundTask,
    .mode                     = MANUAL,
    .previousMode             = MANUAL,
    .status                   = CONNECTED,
};

Blind Blind2 = {
    .id                       = BLIND_2_ID,
    .motorId                  = MOTOR_2_ID,
    .encoderId                = ENCODER_2_ID,
    .alSensorId               = AL_SENSOR_2_ID,
    .ledId                    = LED_ORANGE_ID,
    .switchToDayLightModeTask = switchBlindToDayLightModeTask,
    .blinkLedTask             = &BlinkOrangeLedTask,
    .configSettingsSoundTask  = &configSettingsSoundTask,
    .mode                     = MANUAL,
    .previousMode             = MANUAL,
    .status                   = CONNECTED,
};

/* Private Variable Declarations */
Blind* blinds[NUM_BLINDS] = {&Blind1, &Blind2};
Blind* blindInFocus       = &Blind1;

/* Private Function Prototypes */
void blind_turn_off_selected_blind_signals(void);
void blind_set_blind_in_focus(Blind* blind);
void blind_turn_on_selected_blind_signals(void);

/* Public Functions */

void blind_init(void) {

    blind_update_connection_status();

    if (blindInFocus->status == CONNECTED) {
        blind_turn_on_selected_blind_signals();
    }
}

void blind_update_connection_status(void) {

    for (uint8_t i = 0; i < NUM_BLINDS; i++) {

        // Check if the encoders are connected
        if ((encoder_get_state(blinds[i]->encoderId) == PIN_HIGH) ||
            (bm_attempt_align_encoder(blinds[i]->id) == TRUE)) {
            blinds[i]->status = CONNECTED;

            // Update the blind in focus if it is currently disconnected
            if (blindInFocus->status == DISCONNECTED) {
                blindInFocus = blinds[i];
            }

            continue;
        }

        blinds[i]->status = DISCONNECTED;
    }
}

Blind* blind_get_blind_in_focus(void) {
    return blindInFocus;
}

void bs_set_selected_blind(uint8_t blindId) {

    if (BLIND_ID_INVALID(blindId)) {
        return;
    }

    blind_turn_off_selected_blind_signals();

    // Calculate the index of the blind to set in focus
    uint8_t index = BLIND_ID_TO_INDEX(blindId);
    blind_set_blind_in_focus(blinds[index]);
}

uint8_t blind_get_encoder_id(uint8_t blindId) {
    if (BLIND_ID_INVALID(blindId)) {
        return INVALID_ID;
    }

    uint8_t index = BLIND_ID_TO_INDEX(blindId);
    return blinds[index]->encoderId;
}

uint8_t blind_get_motor_id(uint8_t blindId) {
    if (BLIND_ID_INVALID(blindId)) {
        return INVALID_ID;
    }

    uint8_t index = BLIND_ID_TO_INDEX(blindId);
    return blinds[index]->motorId;
}

uint8_t blind_get_selected_blind_id(void) {
    return blindInFocus->id;
}

uint8_t blind_get_al_sensor_id(uint8_t blindId) {
    if (BLIND_ID_INVALID(blindId)) {
        return INVALID_ID;
    }

    uint8_t index = BLIND_ID_TO_INDEX(blindId);
    return blinds[index]->alSensorId;
}

void blind_set_selected_blind_mode(uint8_t mode) {

    // Turn off all lights and sounds from current mode
    blind_turn_off_selected_blind_signals();
    blindInFocus->mode = mode;
    blind_turn_on_selected_blind_signals();

    // To set the mode back to normal, the encoder min max
    // settings must be verified
    if (blindInFocus->mode == CONFIGURE_SETINGS) {
        bm_set_mode_update_encoder_settings(blindInFocus->id);
    }
}

void blind_revert_selected_blind_mode(void) {
    blind_set_selected_blind_mode(blindInFocus->previousMode);
}

uint8_t blind_get_mode(uint8_t blindId) {

    if (BLIND_ID_INVALID(blindId)) {
        return INVALID_ID;
    }

    uint8_t index = BLIND_ID_TO_INDEX(blindId);
    return blinds[index]->mode;
}

void blind_play_error_sound(void) {
    ts_cancel_running_task(blindInFocus->configSettingsSoundTask);
    piezo_buzzer_play_sound(PIEZO_ERROR_SOUND);
    HAL_Delay(100);
    piezo_buzzer_play_sound(PIEZO_ERROR_SOUND);
    ts_add_task_to_queue(blindInFocus->configSettingsSoundTask);
}

uint8_t blind_get_selected_blind_mode(void) {
    return blindInFocus->mode;
}

void blind_change_selected_blind(void) {

    blind_update_connection_status();

    if ((blindInFocus == &Blind1) && Blind2.status == CONNECTED) {
        blind_turn_off_selected_blind_signals();
        blindInFocus = &Blind2;
        blind_turn_on_selected_blind_signals();
    } else if ((blindInFocus == &Blind2) && Blind1.status == CONNECTED) {
        blind_turn_off_selected_blind_signals();
        blindInFocus = &Blind1;
        blind_turn_on_selected_blind_signals();
    }
}

/* Private Functions */

void blind_turn_off_selected_blind_signals(void) {
    ts_cancel_running_task(blindInFocus->blinkLedTask);
    ts_cancel_running_task(blindInFocus->configSettingsSoundTask);
    led_off(blindInFocus->ledId);
}

void blind_set_blind_in_focus(Blind* blind) {

    blind_turn_on_selected_blind_signals();

    // Update the blind in focus
    blindInFocus = blind;

    blind_turn_on_selected_blind_signals();
}

void blind_turn_on_selected_blind_signals(void) {

    // Start the appropriate signal for the selected blind
    if (blindInFocus->mode == DAY_LIGHT) {
        led_on(blindInFocus->ledId);
        return;
    }

    // Any more that is not DAY_LIGHT will blink the LED
    ts_add_task_to_queue(blindInFocus->blinkLedTask);

    if (blindInFocus->mode == CONFIGURE_SETINGS) {
        ts_add_task_to_queue(blindInFocus->configSettingsSoundTask);
    }
}