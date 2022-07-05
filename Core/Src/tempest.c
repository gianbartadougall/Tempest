/**
 * @file tempest.c
 * @author Gian Barta-Dougall
 * @brief Core logic functions for Project Tempest
 * @version 0.1
 * @date 2022-06-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */
/* Public Includes */

/* Private Includes */
#include "tempest.h"

/* STM32 Includes */

/* Private Enumerations */

enum Mode {MANUAL, AUTOMATIC};

/* Private #defines */
#define UP_PUSH_BUTTON PUSH_BUTTON_0
#define DOWN_PUSH_BUTTON PUSH_BUTTON_1

#define TEMPEST_MODE_MANAUAL 3
#define TEMPEST_MODE_AUTOMATIC 4

#define TEMPEST_MODE_MOTOR_UP 5
#define TEMPEST_MODE_MOTOR_DOWN 6

#define TEMPEST_MOTOR_STOP 7
#define TEMPEST_MOTOR_MOVE_UP 8
#define TEMPEST_MOTOR_MOVE_DOWN 9

#define TEMPEST_BLIND_STATE_UP 10
#define TEMPEST_BLIND_STATE_DOWN 11
#define TEMPEST_CHANGE_MODE 12
#define TEMPEST_MANUAL_OVERRIDE 13
#define TEMPEST_BLIND_STATE_UNDEFINED 14

#define TEMPEST_MOTOR MD_MOTOR_0
#define MOTOR_STOP MOTOR_DRIVER_STOP
#define MOTOR_UP MOTOR_DRIVER_DIRECTION_1
#define MOTOR_DOWN MOTOR_DRIVER_DIRECTION_2

#define MIN_MODE_SWITCH_TIME 1000
#define MIN_PB_TIME 3000

/* Variable Declarations */
enum Mode systemMode;
uint8_t motorState = TEMPEST_MOTOR_STOP;
uint32_t modeSwitchStartTime = 0;
uint8_t manualOverride = 0;
uint8_t manualOverrideMoveUpFlag = 0;
uint8_t manualOverrideMoveDownFlag = 0;

TimerMsHandle modeSwitchHandle;
TimerMsHandle ambientLightSensorHandle;

/* Function prototypes */
void tempest_error_handler(void);
uint8_t tempest_decode_input_state(void);
void tempest_stop_motor(void);
void tempest_set_motor_up(void);
void tempest_set_motor_down(void);

/**
 * @brief ISR to be called when a mode switch needs to occur. Function sets
 * mode to manual if it's currently automatic and sets mode to automatic if
 * it's currently manual. Updates LED status based on new mode. Plays sound
 * on piezo buzzer to indicate mode switch has taken place
 */
void tempest_mode_switch_isr(void) {

    switch (systemMode) {
        case MANUAL:
            systemMode = AUTOMATIC;
            piezo_buzzer_play_sound(sound1);
            brd_led_on(); // Update LED
            break;
        case AUTOMATIC:
            systemMode = MANUAL;
            piezo_buzzer_play_sound(sound1);
            brd_led_off(); // Update LED
            break;
        default:
            debug_prints("ERROR: Unknown system mode\r\n");
            tempest_stop_motor();
            tempest_error_handler();
            break;
    }
}

void tempest_update_system_state(void) {

    // Read the state of the pushbuttons
    uint8_t inputState = tempest_decode_input_state();

    switch (inputState) {

        case TEMPEST_CHANGE_MODE:

            // Do nothing if the system is in manual override
            if (manualOverride == 1) {
                return;
            }

            if (timer_ms_handle_in_use(&modeSwitchHandle) == 0) {
                timer_ms_set_delay(&modeSwitchHandle); // Ignoring result as there will always be a free channel
                debug_prints("timer delay set for 3 seconds\r\n");
            }

            return;

        case TEMPEST_MOTOR_MOVE_UP:

            if (manualOverride == 1) {
                
                // Move motor up if button has been pressed for long enough
                if (manualOverrideMoveUpFlag == 1) {
                    tempest_set_motor_up();
                    
                    // debug_prints("Forcing blind up\r\n");
                }

                break;
            }

            if (!encoder_at_minimum_distance()) {
                tempest_set_motor_up();
            }

            break;
        
        case TEMPEST_MOTOR_MOVE_DOWN:

            if (manualOverride == 1) {
                
                // Move motor down if button has been pressed for long enough
                if (manualOverrideMoveDownFlag == 1) {
                    // debug_prints("Forcing blind down\r\n");
                    tempest_set_motor_down();
                }

                break;
            }

            if (!encoder_at_maximum_distance()) {
                tempest_set_motor_down();
            }

            break;
        
        case TEMPEST_MOTOR_STOP:

            if (manualOverride == 1) {
                
                // Stop the motor
                tempest_stop_motor();
            }

            if (systemMode == MANUAL) {
                tempest_stop_motor();
            }

            if (systemMode == AUTOMATIC) {
                
                // If the delay has ended, set a new delay sequence
                if (timer_ms_handle_in_use(&ambientLightSensorHandle) == 0) {

                    // Disregard return value as only two handles are used in this system
                    timer_ms_set_delay(&ambientLightSensorHandle);
                    
                    // Read ambient light sensor value
                    uint8_t lightLevel = ambient_light_read();
                    
                    // Set the motor to the correct hight
                    if ((lightLevel == HIGH) && !encoder_at_minimum_distance()) {
                        debug_prints("Setting motor up\r\n");
                        tempest_set_motor_up();
                    } else if ((lightLevel == LOW) && !encoder_at_maximum_distance()) {
                        tempest_set_motor_down();
                        debug_prints("Setting motor down\r\n");
                    } else {
                        debug_prints("Light level Undetermined\r\n");
                    }
                }

            }

            break;
    }

    // Cancel mode switch delay if delay is currently running but both pushbuttons are not pressed
    if (timer_ms_handle_in_use(&modeSwitchHandle) == 1) {
        timer_ms_cancel_delay(&modeSwitchHandle);
    }

}

uint8_t tempest_decode_input_state(void) {

    // Get the states of the buttons
    uint8_t pbUpState = pb_get_state(UP_PUSH_BUTTON);
    uint8_t pbDownState = pb_get_state(DOWN_PUSH_BUTTON);

    // char msg[40];
    // sprintf(msg, "%i %i\r\n", pbUpState, pbDownState);
    // debug_prints(msg);

    if ((pbUpState == 1) && (pbDownState == 1)) {
        return TEMPEST_CHANGE_MODE;
    }

    if ((pbUpState == 1) && (pbDownState == 0)) {
        return TEMPEST_MOTOR_MOVE_UP;
    }

    if ((pbUpState == 0) && (pbDownState == 1)) {
        return TEMPEST_MOTOR_MOVE_DOWN;
    }

    if ((pbUpState == 0) && (pbDownState == 0)) {
        return TEMPEST_MOTOR_STOP;
    }

    return TEMPEST_BLIND_STATE_UNDEFINED;
}

void tempest_stop_motor(void) {
    // debug_prints("Stopping motor\r\n");
    if (motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_STOP) != HAL_OK) {
        tempest_error_handler();
    }

    // Clear manual override flags
    manualOverrideMoveUpFlag = 0;
    manualOverrideMoveDownFlag = 0;
}

void tempest_set_motor_up(void) {
    debug_prints("Motor UP\r\n");
    // Set the direction of the encoder
    encoder_set_direction_negative();

    if (motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_UP) != HAL_OK) {
        tempest_error_handler();
    }
}

void tempest_set_motor_down(void) {
    debug_prints("Motor DOWN\r\n");
    // Set the direction of the encoder
    encoder_set_direction_positive();

    if (motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_DOWN) != HAL_OK) {
        tempest_error_handler();
    }
}

void tempest_hardware_init(void) {
    // Initialise LED
	board_init();

	// Initialise motor driver
	motor_driver_init();
    
	// Initialise rotary encoder
    encoder_init();

	// Initialise pushbuttons
	pb_init();

    // Initialise generic millisecond timer
    timer_ms_init();
    timer_ms_enable(); // Enable timer
    
    // Initialise the handles for required delays
    void (*mshISRList[1])(void) = {&tempest_mode_switch_isr};
    uint16_t mshDelays[1] = {3000};
    timer_ms_init_handle(&modeSwitchHandle, mshISRList, mshDelays, 1);
    
    void (*alsISRList[3])(void) = {&ambient_light_sensor_isr_s1, 
                                    &ambient_light_sensor_isr_s2,
                                    &ambient_light_sensor_isr_s3};
    uint16_t alsDelays[3] = {60000, 1, 1000};
    timer_ms_init_handle(&ambientLightSensorHandle, alsISRList, alsDelays, 3);

    // Initialise piezo buzzer
    piezo_buzzer_init();

    // Initialise flag to act as manual override pin
    flag_init();

    // Set system mode to automatic
    systemMode = AUTOMATIC;
}

void tempest_error_handler(void) {

    while (1) {
        piezo_buzzer_play_sound(sound1);
        HAL_Delay(100);
    }
}

void play_buzzer(uint8_t length) {
    piezo_buzzer_play_sound(sound2);
    HAL_Delay(length);
    piezo_buzzer_play_sound(sound2);
    HAL_Delay(length);
    piezo_buzzer_play_sound(sound2);
}

/* ISRs */

void tempest_isr_set_manual_override(void) {
    debug_prints("Manual override set\r\n");
    // Set override mode flag
    manualOverride = 1;

    // Stop the motor
    tempest_stop_motor();

    // Enter manual override mode in the encoder
    encoder_enter_manual_override();

    // Start the timer on the pushbuttons
    pb_start_timer();

    // Play piezeo buzzer sound to alert user
    play_buzzer(50);
}

void tempest_isr_clear_manual_override(void) {
    debug_prints("Manual override cleared\r\n");
    // Clear override mode flag
    manualOverride = 0;

    // Exit manual override mode in the encoder
    encoder_exit_manual_override();

    // Stop push button timer
    pb_stop_timer();
}

void tempest_isr_encoder_at_min_value(void) {
    tempest_stop_motor(); // Stop the motor
}

void tempest_isr_encoder_at_max_value(void) {
    tempest_stop_motor(); // Stop the motor
}

void tempest_isr_force_blind_up(void) {
    
    // Confirm manual override is set
    if (manualOverride == 1) {
        manualOverrideMoveUpFlag = 1; // Set flag to allow blind to move up
    }
}

void tempest_isr_force_blind_down(void) {
    
    // Confirm manual override is set
    if (manualOverride == 1) {
        manualOverrideMoveDownFlag = 1; // Set flag to allow blind to move down
    }
}