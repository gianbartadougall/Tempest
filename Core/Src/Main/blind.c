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
#include "blind_motor.h"
#include "ambient_light_sensor.h"
#include "utilities.h"
#include "user_interface.h"
#include "piezo_buzzer.h"
#include "blind_motor.h"
#include "led.h"
#include "hardware_config.h"

/* Private STM Includes */

/* Private #defines */

#define BLIND_ID_INVALID(id)  ((id < BLIND_ID_OFFSET) || (id > (NUM_BLINDS - 1 + BLIND_ID_OFFSET)))
#define BLIND_ID_TO_INDEX(id) (id - BLIND_ID_OFFSET)

#define ASSERT_VALID_BLIND_ID(id)                                                       \
    do {                                                                                \
        if ((id < BLIND_ID_OFFSET) || (id > (NUM_BLINDS - 1 + BLIND_ID_OFFSET))) {      \
            char msg[100];                                                              \
            sprintf(msg, "Invalid ID: File %s line number %d\r\n", __FILE__, __LINE__); \
            log_prints(msg);                                                            \
            return;                                                                     \
        }                                                                               \
    } while (0)

#define ASSERT_VALID_BLIND_ID_RETVAL(id, retval)                                        \
    do {                                                                                \
        if ((id < BLIND_ID_OFFSET) || (id > (NUM_BLINDS - 1 + BLIND_ID_OFFSET))) {      \
            char msg[100];                                                              \
            sprintf(msg, "Invalid ID: File %s line number %d\r\n", __FILE__, __LINE__); \
            log_prints(msg);                                                            \
            return retval;                                                              \
        }                                                                               \
    } while (0)

/* Private Structures and Enumerations */

const struct Task1 switchBlindToDayLightModeTask = {
    .delay      = 5000,
    .functionId = FUNC_ID_SWITCH_BLIND_MODE_TO_DAYLIGHT,
    .group      = BLIND_GROUP,
    .nextTask   = NULL,
};

Blind Blind1 = {
    .id                       = BLIND_1_ID,
    .blindMotorId             = BLIND_MOTOR_1_ID,
    .alSensorId               = AL_SENSOR_1_ID,
    .userInterfaceId          = USER_INTERFACE_1_ID,
    .switchToDayLightModeTask = switchBlindToDayLightModeTask,
    .mode                     = DAY_LIGHT,
    .previousMode             = DAY_LIGHT,
    .status                   = DISCONNECTED,
};

Blind Blind2 = {
    .id                       = BLIND_2_ID,
    .blindMotorId             = BLIND_MOTOR_2_ID,
    .alSensorId               = AL_SENSOR_2_ID,
    .userInterfaceId          = USER_INTERFACE_2_ID,
    .switchToDayLightModeTask = switchBlindToDayLightModeTask,
    .mode                     = DAY_LIGHT,
    .previousMode             = DAY_LIGHT,
    .status                   = DISCONNECTED,
};

/* Private Variable Declarations */
Blind* blinds[NUM_BLINDS] = {&Blind1, &Blind2};
Blind* blindInFocus       = &Blind1;
extern uint32_t blindTasksFlag;

/* Private Function Prototypes */
void blind_set_bif(Blind* blind);
void blind_update_user_interface(void);
void blind_update_connections_status(void);
void blind_user_interface_on(Blind* blind);

/* Public Functions */

void blind_init(void) {
    blind_motor_init();
    blind_update_connections_status();
}

void blind_print_info(uint8_t blindId) {
    char info[];
    sprintf(info,
            "Operational: %s\r\n"
            "Height: %i/%i\r\n",
            blinds[i].status == CONNECTED ? "yes" : "no", bm_get_height(blinds[blindId]->blindMotorId), );
}

void blind_update_connections_status(void) {

    // Because the blind in focus is a pointer, it is necessary to store
    // it's current value before probing the blind connections so it can
    // be determined if blind in focus needs to be updated or not
    uint8_t currentBIFStatus = blindInFocus->status;

    // Check the blind motor connection status of all blinds and update
    // their status accordingly. If the current Blind In Focus is disconnected,
    // assign the Blind In Focus to the first blind whose status is connected
    for (uint8_t i = 0; i < NUM_BLINDS; i++) {

        blinds[i]->status = bm_probe_connection(blinds[i]->blindMotorId);

        if (currentBIFStatus == DISCONNECTED && blinds[i]->status == CONNECTED) {
            blind_set_bif(blinds[i]);
            currentBIFStatus = CONNECTED;
        }
    }
}

void blind_move_up(uint8_t blindId) {

    ASSERT_VALID_BLIND_ID(blindId);
    uint8_t index = BLIND_ID_TO_INDEX(blindId);

    bm_move_blind(blinds[index]->blindMotorId, BLIND_UP);
}

void blind_move_down(uint8_t blindId) {
    ASSERT_VALID_BLIND_ID(blindId);
    uint8_t index = BLIND_ID_TO_INDEX(blindId);

    bm_move_blind(blinds[index]->blindMotorId, BLIND_DOWN);
}

void blind_stop_moving(uint8_t blindId) {
    ASSERT_VALID_BLIND_ID(blindId);
    uint8_t index = BLIND_ID_TO_INDEX(blindId);

    bm_stop_blind_moving(blinds[index]->blindMotorId);
}

void blind_stop_bif_moving(void) {
    bm_stop_blind_moving(blindInFocus->blindMotorId);
}

void blind_set_new_min_height(uint8_t blindId) {
    ASSERT_VALID_BLIND_ID(blindId);
    uint8_t index = BLIND_ID_TO_INDEX(blindId);
    bm_set_new_min_height(blinds[index]->blindMotorId);
}

void blind_set_new_max_height(uint8_t blindId) {
    ASSERT_VALID_BLIND_ID(blindId);
    uint8_t index = BLIND_ID_TO_INDEX(blindId);
    bm_set_new_max_height(blinds[index]->blindMotorId);
}

uint8_t blind_min_max_heights_are_valid(uint8_t blindId) {
    ASSERT_VALID_BLIND_ID_RETVAL(blindId, FALSE);
    uint8_t index = BLIND_ID_TO_INDEX(blindId);
    return bm_min_max_heights_are_valid(blinds[index]->blindMotorId);
}

Blind* blind_get_bif(void) {
    return blindInFocus;
}

void bs_set_new_bif(uint8_t blindId) {

    if (BLIND_ID_INVALID(blindId)) {
        return;
    }

    user_interface_off(blindInFocus->userInterfaceId);

    // Calculate the index of the blind to set in focus
    uint8_t index = BLIND_ID_TO_INDEX(blindId);
    blindInFocus  = blinds[index];

    blind_update_user_interface();
}

uint8_t blind_get_bif_id(void) {
    return blindInFocus->id;
}

void blind_set_bif_mode(uint8_t mode) {

    // Turn off all lights and sounds from current mode
    user_interface_off(blindInFocus->userInterfaceId);

    blindInFocus->mode = mode;

    // The task to change to day light mode needs to be cancelled if manual
    // mode is selected in case the user clicked day light mode and then
    // clicked manual mode before the daylight mode could take affect
    if (blindInFocus->mode == BLIND_MODE_MANUAL) {
        ts_cancel_running_task(&blindInFocus->switchToDayLightModeTask);
    }

    // To set the mode back to normal, the encoder min max
    // settings must be verified
    if (blindInFocus->mode == CONFIGURE_SETINGS) {
        bm_set_mode_update_encoder_settings(blindInFocus->blindMotorId);
    }

    if (blindInFocus->mode == BLIND_MODE_DAY_LIGHT) {
        log_prints("Now set to daylight\r\n");
    }

    blind_user_interface_on(blindInFocus);
}

void blind_revert_bif_mode(void) {
    blind_set_bif_mode(blindInFocus->previousMode);
}

uint8_t blind_get_mode(uint8_t blindId) {

    if (BLIND_ID_INVALID(blindId)) {
        return INVALID_ID;
    }

    uint8_t index = BLIND_ID_TO_INDEX(blindId);
    return blinds[index]->mode;
}

void blind_play_error_sound(void) {}

uint8_t blind_get_bif_mode(void) {
    return blindInFocus->mode;
}

void blind_toggle_bif(void) {

    blind_update_connections_status();

    if ((blindInFocus == &Blind1) && Blind2.status == CONNECTED) {
        blind_set_bif(&Blind2);
    } else if ((blindInFocus == &Blind2) && Blind1.status == CONNECTED) {
        blind_set_bif(&Blind1);
    }
}

/* Private Functions */

void blind_set_bif(Blind* blind) {

    user_interface_off(blindInFocus->userInterfaceId);

    // Update the blind in focus
    blindInFocus = blind;

    log_prints("UI on\r\n");
    blind_user_interface_on(blindInFocus);
}

void blind_user_interface_on(Blind* blind) {

    switch (blind->mode) {
        case BLIND_MODE_MANUAL:
            user_interface_manual_mode_on(blind->userInterfaceId);
            break;
        case BLIND_MODE_DAY_LIGHT:
            user_interface_daylight_mode_on(blind->userInterfaceId);
            break;
        case CONFIGURE_SETINGS:
            user_interface_update_settings_mode_on(blind->userInterfaceId);
            break;
        default:
            break;
    }
}

void blind_process_internal_flags(void) {

    if (FLAG_IS_SET(blindTasksFlag, FUNC_ID_BLINK_RED_LED)) {
        FLAG_CLEAR(blindTasksFlag, FUNC_ID_BLINK_RED_LED);
        led_toggle(LED_RED_ID);
    }

    if (FLAG_IS_SET(blindTasksFlag, FUNC_ID_BLINK_ORANGE_LED)) {
        FLAG_CLEAR(blindTasksFlag, FUNC_ID_BLINK_ORANGE_LED);
        led_toggle(LED_ORANGE_ID);
    }

    if (FLAG_IS_SET(blindTasksFlag, FUNC_ID_SWITCH_BLIND_MODE_TO_DAYLIGHT)) {
        FLAG_CLEAR(blindTasksFlag, FUNC_ID_SWITCH_BLIND_MODE_TO_DAYLIGHT);
        blindInFocus->mode = DAY_LIGHT;
    }

    if (FLAG_IS_SET(blindTasksFlag, FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND)) {
        FLAG_CLEAR(blindTasksFlag, FUNC_ID_PLAY_CONFIG_SETTINGS_SOUND);
        piezo_buzzer_play_sound(SOUND);
    }

    if (FLAG_IS_SET(blindTasksFlag, FUNC_ID_BLINDS_DAY_LIGHT_UPDATE)) {
        FLAG_CLEAR(blindTasksFlag, FUNC_ID_BLINDS_DAY_LIGHT_UPDATE);
        // log_prints("Light update occured\r\n");

        for (uint8_t i = 0; i < NUM_BLINDS; i++) {

            if (blinds[i]->mode != DAY_LIGHT) {
                continue;
            }

            if (PIN_IDR_IS_HIGH(HC_ALS_PORT_1, HC_ALS_PIN_1)) {
                bm_move_blind(blinds[i]->blindMotorId, BLIND_UP);
            } else {
                bm_move_blind(blinds[i]->blindMotorId, BLIND_DOWN);
            }

            if (al_sensor_status(blinds[i]->alSensorId) == DISCONNECTED) {
                continue;
            }

            // uint8_t lightFound = al_sensor_light_found(blinds[i]->alSensorId);
            // // char m[60];
            // // sprintf(m, "Light found = %s\r\n", lightFound == 1 ? "TRUE" : "FALSE");
            // // log_prints(m);

            // if (lightFound == TRUE && (bm_blind_at_max_height(blinds[i]->blindMotorId) == FALSE)) {
            //     bm_move_blind(blinds[i]->blindMotorId, BLIND_UP);
            // }

            // if (lightFound == FALSE && (bm_blind_at_min_height(blinds[i]->blindMotorId) == FALSE)) {
            //     bm_move_blind(blinds[i]->blindMotorId, BLIND_DOWN);
            // }
        }
    }

    // Process internal flags of lower level abstraction layers
    bm_process_internal_flags();
}