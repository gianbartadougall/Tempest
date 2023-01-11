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
#include "synchronous_timer.h"
#include "button.h"
#include "blind.h"

/* Private STM Includes */

/* Private Macros */

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
    log_clear();

    // Initialise all the required peripherals
    ts_init();
    button_init();
    blind_init();
    synchronous_timer_enable();

    log_prints("Initialised\r\n");
}

/* Private Functions */

void tempest_update(void) {

    /* Ensures button debouncing is processed */
    button_process_internal_flags();

    /* Task scheduler internal flags need to be processed as they
        handle things like removing tasks that have been completed
        from the scheduler and running new tasks */
    ts_process_internal_flags();

    /* Process any buttons clicks/holds by the user */
    tempest_process_external_button_flags();

    /* Updates things like LEDs that turn on/off depending on what mode
        the blinds are in, process flags for automatic raising/lowering
        of the blinds */
    blind_process_internal_flags();

    /* The ambient light sensor does not currently work properly
        so it is not being run */
    al_sensor_process_internal_flags();
}

void tempest_process_external_button_flags(void) {

    // Switch the current selected blind
    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD) &&
        FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD)) {

        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD);
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD);

        blind_toggle_bif();
    }

    /****** START CODE BLOCK ******/
    // Description: Process single clicks from both button up and down

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_SINGLE_CLICK)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_SINGLE_CLICK);

        uint8_t blindId = blind_get_bif_id();

        if (blind_get_bif_mode() == MANUAL) {
            log_prints("Moving blind up\r\n");
            blind_move_up(blindId);
        }

        if (blind_get_bif_mode() == CONFIGURE_SETINGS) {
            log_prints("Setting max height\r\n");
            blind_set_new_max_height(blindId);
        }
    }

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_SINGLE_CLICK)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_SINGLE_CLICK);

        uint8_t blindId = blind_get_bif_id();

        if (blind_get_bif_mode() == MANUAL) {
            log_prints("Moving blind down\r\n");
            blind_move_down(blindId);
        }

        if (blind_get_bif_mode() == CONFIGURE_SETINGS) {
            log_prints("Setting min height\r\n");
            blind_set_new_min_height(blindId);
        }
    }

    /****** END CODE BLOCK ******/

    /****** START CODE BLOCK ******/
    // Description: Process double clicks from both button up and button down

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_DOUBLE_CLICK)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_DOUBLE_CLICK);

        uint8_t blindId = blind_get_bif_id();
        blind_stop_bif_moving();

        switch (blind_get_bif_mode()) {
            case MANUAL:
            case DAY_LIGHT:
                blind_set_bif_mode(CONFIGURE_SETINGS);
                break;
            case CONFIGURE_SETINGS:

                if (blind_min_max_heights_are_valid(blindId) == TRUE) {
                    log_prints("Max height valid!!!\r\n");
                    blind_revert_bif_mode();
                } else {
                    log_prints("Max height invalid\r\n");
                    blind_play_error_sound();
                }

                break;
            default:
                break;
        }
    }

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_DOUBLE_CLICK)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_DOUBLE_CLICK);

        uint8_t blindId = blind_get_bif_id();
        blind_stop_moving(blindId);

        /* Using a switch statement here instead of an if statement because
            it's easy to forget that you cannot have two seperate if statements
            and that it must be an if else otherwise once the blind is set to
            daylight it will reset to manual straight away in the next if statement*/
        switch (blind_get_bif_mode()) {
            case MANUAL:
                blind_set_bif_mode(DAY_LIGHT);
                break;
            case DAY_LIGHT:
                blind_set_bif_mode(MANUAL);
                break;
            default:
                break;
        }
    }

    // /****** END CODE BLOCK ******/

    // /****** START CODE BLOCK ******/
    // // Description: Process press and holds from both button up and button down

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD);

        if ((blind_get_bif_mode() == MANUAL) || (blind_get_bif_mode() == CONFIGURE_SETINGS)) {
            blind_move_up(blind_get_bif_id());
            log_prints("Move blind up\r\n");
        }
    }

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD);

        if ((blind_get_bif_mode() == MANUAL) || (blind_get_bif_mode() == CONFIGURE_SETINGS)) {
            blind_move_down(blind_get_bif_id());
            log_prints("Move blind down\r\n");
        }
    }

    // /****** END CODE BLOCK ******/

    // /****** START CODE BLOCK ******/
    // // Description: Process the release of a press and hold from both button up and button down

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD_RELEASED)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_UP_PRESS_AND_HOLD_RELEASED);

        if ((blind_get_bif_mode() == MANUAL) || (blind_get_bif_mode() == CONFIGURE_SETINGS)) {
            blind_stop_bif_moving();
            // bm_stop_blind_moving(blind_get_bif_id());
            log_prints("Stop moving blind\r\n");
        }
    }

    if (FLAG_IS_SET(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD_RELEASED)) {
        FLAG_CLEAR(buttonTasksFlag, FUNC_ID_BUTTON_DOWN_PRESS_AND_HOLD_RELEASED);

        if ((blind_get_bif_mode() == MANUAL) || (blind_get_bif_mode() == CONFIGURE_SETINGS)) {
            blind_stop_bif_moving();
            // bm_stop_blind_moving(blind_get_bif_id());
            log_prints("Stop moving blind\r\n");
        }
    }

    /****** END CODE BLOCK ******/
}