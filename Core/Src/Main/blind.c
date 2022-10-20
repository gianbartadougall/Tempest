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

/* Private STM Includes */

/* Private #defines */
#define BLIND_ID_INVALID(id)  ((id < BLIND_ID_OFFSET) || (id > (NUM_BLINDS - 1 + BLIND_ID_OFFSET)))
#define BLIND_ID_TO_INDEX(id) (id - BLIND_ID_OFFSET)

/* Private Structures and Enumerations */

enum TempestFunctions {
    FUNC_ID_SWITCH_BLIND_1_MODE_TO_DAYLIGHT,
    FUNC_ID_SWITCH_BLIND_2_MODE_TO_DAYLIGHT,
    FUNC_ID_BLINK_RED_LED,
    FUNC_ID_BLINK_ORANGE_LED,
    FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND,
    FUNC_ID_PRINT_TIMER_COUNT,
    FUNC_ID_CONFIRM_ENCODER_1_IN_OPERATION,
    FUNC_ID_CONFIRM_ENCODER_2_IN_OPERATION,
};

struct Task1 printTimerCount = {
    .delay      = 500,
    .functionId = FUNC_ID_PRINT_TIMER_COUNT,
    .group      = TEMPEST_GROUP,
    .nextTask   = &printTimerCount,
};

struct Task1 encoder1InOperation = {
    .delay      = 500,
    .functionId = FUNC_ID_CONFIRM_ENCODER_1_IN_OPERATION,
    .group      = TEMPEST_GROUP,
    .nextTask   = &encoder1InOperation,
};

struct Task1 encoder2InOperation = {
    .delay      = 500,
    .functionId = FUNC_ID_CONFIRM_ENCODER_2_IN_OPERATION,
    .group      = TEMPEST_GROUP,
    .nextTask   = &encoder2InOperation,
};

const struct Task1 switchBlind1ToDayLightModeTask = {
    .delay      = 5000,
    .functionId = FUNC_ID_SWITCH_BLIND_1_MODE_TO_DAYLIGHT,
    .group      = TEMPEST_GROUP,
    .nextTask   = NULL,
};

const struct Task1 switchBlind2ToDayLightModeTask = {
    .delay      = 5000,
    .functionId = FUNC_ID_SWITCH_BLIND_2_MODE_TO_DAYLIGHT,
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
    .id                        = BLIND_1_ID,
    .motorId                   = MOTOR_1_ID,
    .encoderId                 = ENCODER_1_ID,
    .alSensorId                = AL_SENSOR_1_ID,
    .ledId                     = LED_RED_ID,
    .switchToDayLightModeTask  = switchBlind1ToDayLightModeTask,
    .confirmEncoderInOperation = &encoder1InOperation,
    .blinkLedTask              = &blinkRedLedTask,
    .configSettingsSoundTask   = &configSettingsSoundTask,
    .mode                      = MANUAL,
    .previousMode              = MANUAL,
    .status                    = CONNECTED,
    .lastEncoderCount          = 0,
};

Blind Blind2 = {
    .id                        = BLIND_2_ID,
    .motorId                   = MOTOR_2_ID,
    .encoderId                 = ENCODER_2_ID,
    .alSensorId                = AL_SENSOR_2_ID,
    .ledId                     = LED_ORANGE_ID,
    .switchToDayLightModeTask  = switchBlind2ToDayLightModeTask,
    .confirmEncoderInOperation = &encoder2InOperation,
    .blinkLedTask              = &BlinkOrangeLedTask,
    .configSettingsSoundTask   = &configSettingsSoundTask,
    .mode                      = MANUAL,
    .previousMode              = MANUAL,
    .status                    = CONNECTED,
    .lastEncoderCount          = 0,
};

/* Private Variable Declarations */
Blind* blinds[NUM_BLINDS] = {&Blind1, &Blind2};
Blind* blindInFocus       = &Blind1;

/* Private Function Prototypes */
void blind_unselect_blind_in_focus(void);
void blind_set_blind_in_focus(Blind* blind);

/* Public Functions */

void blind_update_connection_status(void) {

    for (uint8_t i = 0; i < NUM_BLINDS; i++) {

        // Check if the encoders are connected
        if ((encoder_get_state(blinds[i]->encoderId) == PIN_HIGH) ||
            bm_attempt_align_encoder(blinds[i]->encoderId) == TRUE) {
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

    blind_unselect_blind_in_focus();

    // Calculate the index of the blind to set in focus
    uint8_t index = BLIND_ID_TO_INDEX(blindId);
    blind_set_blind_in_focus(blinds[index]);
}

void blind_cancel_encoder_checking_task(uint8_t blindId) {
    if (BLIND_ID_INVALID(blindId)) {
        return;
    }

    uint8_t index = BLIND_ID_TO_INDEX(blindId);
    ts_cancel_running_task(blinds[index]->confirmEncoderInOperation);
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
    return blinds[index]->encoderId;
}

/* Private Functions */

void blind_unselect_blind_in_focus(void) {
    ts_cancel_running_task(blindInFocus->blinkLedTask);
    ts_cancel_running_task(blindInFocus->configSettingsSoundTask);
    led_off(blindInFocus->ledId);
}

void blind_set_blind_in_focus(Blind* blind) {

    // Update the blind in focus
    blindInFocus = blind;

    // Start the appropriate signal for the selected blind
    if (blind->mode == DAY_LIGHT) {
        led_on(blind->ledId);
        return;
    }

    // Any more that is not DAY_LIGHT will blink the LED
    ts_add_task_to_queue(blind->blinkLedTask);

    if (blind->mode == CONFIGURE_SETINGS) {
        ts_add_task_to_queue(blind->configSettingsSoundTask);
    }
}