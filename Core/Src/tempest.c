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

/* Private #defines */
#define UP_PUSH_BUTTON PUSH_BUTTON_0
#define DOWN_PUSH_BUTTON PUSH_BUTTON_1

#define TEMPEST_MODE_SWITCH 2
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
uint8_t mode = TEMPEST_MODE_AUTOMATIC;
uint8_t motorState = TEMPEST_MOTOR_STOP;
uint32_t modeSwitchStartTime = 0;
uint8_t manualOverride = 0;

/* Function prototypes */
void tempest_error_handler(void);
uint8_t tempest_decode_system_state(void);
void tempest_stop_motor(void);
void tempest_set_motor_up(void);
void tempest_set_motor_down(void);

void tempest_update_system_state(void) {

    // Read the state of the pushbuttons
    uint8_t systemState = tempest_decode_system_state();

    switch (systemState) {

        case TEMPEST_CHANGE_MODE:

            // Do nothing if the system is in manual override
            if (manualOverride == 1) {
                return;
            }
            
            // Update the system mode if the mode is not in manual override
            tempest_update_mode();

            return;

        case TEMPEST_MOTOR_MOVE_UP:

            if (manualOverride == 1) {
                

                break;
            }

            if (!encoder_at_minimum_distance() || manualOverride == 1) {
                tempest_set_motor_up();
            }

            break;
        
        case TEMPEST_MOTOR_MOVE_DOWN:

            if (manualOverride == 1) {
                
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

            if (mode == TEMPEST_MODE_MANAUAL) {
                tempest_stop_motor();
            }

            break;
    }

    // Reset the timer
    modeSwitchStartTime = 0;
}

uint8_t tempest_decode_system_state(void) {

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

void set_manual_override(void) {
    debug_prints("MAN OVERRIDE NOW 1\r\n");
    manualOverride = 1;
    piezo_buzzer_play_sound(sound2);
    HAL_Delay(50);
    piezo_buzzer_play_sound(sound2);
    HAL_Delay(50);
    piezo_buzzer_play_sound(sound2);
}

void clear_manual_override(void) {
    debug_prints("MAN OVERRIDE NOW 0\r\n");
    manualOverride = 0;
    encoder_reset();
    piezo_buzzer_play_sound(sound2);
    HAL_Delay(10);
    piezo_buzzer_play_sound(sound2);
    HAL_Delay(10);
    piezo_buzzer_play_sound(sound2);
}

void tempest_stop_motor(void) {
    if (motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_STOP) != HAL_OK) {
        tempest_error_handler();
    }
}

void tempest_set_motor_up(void) {

    // Set the direction of the encoder
    encoder_set_direction_negative();

    if (motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_UP) != HAL_OK) {
        tempest_error_handler();
    }
}

void tempest_set_motor_down(void) {

    // Set the direction of the encoder
    encoder_set_direction_positive();

    if (motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_DOWN) != HAL_OK) {
        tempest_error_handler();
    }
}

void tempest_update_motor_state(void) {
    
    switch (motorState) {

        case TEMPEST_MOTOR_STOP:

            tempest_stop_motor();
            break;
        
        case TEMPEST_MOTOR_MOVE_UP:

            tempest_set_motor_up();
            break;
        
        case TEMPEST_MOTOR_MOVE_DOWN:

            tempest_set_motor_down();
            break;

        default:
            break;
    }
  
  
  
    if (motorState == TEMPEST_MOTOR_STOP) {
        tempest_stop_motor();
        return;
    }

    if (motorState == TEMPEST_MOTOR_MOVE_UP) {

        // If manual override is on, move motor
        if (manualOverride == 1) {
            tempest_set_motor_up();
            return;
        }

        // Confirm the motor can move up
        if (encoder_at_minimum_distance()) {
            // Stop motor because the blind has reached the minimum distance
            tempest_stop_motor();
        } else {
            tempest_set_motor_up();
        }

        return;
    }

    if (motorState == TEMPEST_MOTOR_MOVE_DOWN) {

        // If manual override is on, move motor
        if (manualOverride == 1) {
            tempest_set_motor_down();
            return;
        }

        // Confirm the motor can move down
        if (encoder_at_maximum_distance()) {
            // Stop motor because the blind has reached the maximum distance
            tempest_stop_motor();
        } else {
            tempest_set_motor_down();
        }

        return;
    }
}

void tempest_update_mode(void) {

    // Turn the motor off
    tempest_stop_motor();

    // Set start time if it has not been set
    if (modeSwitchStartTime == 0) {

        if ((HAL_MAX_DELAY - MIN_MODE_SWITCH_TIME) < HAL_GetTick()) {
            // Wait for timer to reset if minimum hold time cannot be acheived
            while ((HAL_MAX_DELAY - MIN_MODE_SWITCH_TIME) < HAL_GetTick()) {}
        }

        modeSwitchStartTime = HAL_GetTick();
    }

    // Perform mode switch if enough time has passed
    if ((HAL_GetTick() - modeSwitchStartTime) > MIN_MODE_SWITCH_TIME) {

        if (mode == TEMPEST_MODE_AUTOMATIC) {
            mode = TEMPEST_MODE_MANAUAL;
            piezo_buzzer_play_sound(sound1);
        } else if (mode == TEMPEST_MODE_MANAUAL) {
            mode = TEMPEST_MODE_AUTOMATIC;
            piezo_buzzer_play_sound(sound1);
        }

        // Reset mode switch start time
        modeSwitchStartTime = 0;
    }

}


void tempest_update_mode_indicator(void) {

    if (mode == TEMPEST_MODE_MANAUAL) {
        brd_led_on();
    }

    if (mode == TEMPEST_MODE_AUTOMATIC) {
        brd_led_off();
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

	// Initialise ambient light sensor
	ambient_light_sensor_init();

    // Initialise piezo buzzer
    piezo_buzzer_init();

    // Initialise flag to act as manual override pin
    flag_init();
}

void tempest_print_system_state(void) {
    
    char msg[40];
    
    switch (mode) {
    case TEMPEST_MODE_SWITCH:
        sprintf(msg, "System State: Mode switch\r\n");
        break;
    case TEMPEST_MODE_MANAUAL:
        sprintf(msg, "System State: Mode Manual\r\n");
        break;
    case TEMPEST_MODE_AUTOMATIC:
        sprintf(msg, "System State: Mode Automatic\r\n");
        break;
    default:
        sprintf(msg, "System State: Undefined\r\n");
        break;
    }
    
    debug_prints(msg);
}

void tempest_print_motor_state(void) {
    
    char msg[40];
    
    switch (mode) {
    case TEMPEST_MOTOR_STOP:
        sprintf(msg, "Motor State: Stop\r\n");
        break;
    case TEMPEST_MOTOR_MOVE_UP:
        sprintf(msg, "Motor State: Move Up\r\n");
        break;
    case TEMPEST_MOTOR_MOVE_DOWN:
        sprintf(msg, "Motor State: Move Down\r\n");
        break;
    default:
        sprintf(msg, "Motor State: Undefined\r\n");
        break;
    }
    
    debug_prints(msg);
}

void tempest_error_handler(void) {

    while (1) {
        piezo_buzzer_play_sound(sound1);
        HAL_Delay(100);
    }

}

/* ISRs */

void tempest_isr_set_manual_override(void) {

    // Set override mode flag
    manualOverride = 1;

    // Stop the motor
    tempest_stop_motor();

    // Enter manual override mode in the encoder
    encoder_enter_manual_override();

    // Start the timer on the pushbuttons
    pb_start_timer();
}

void tempest_isr_clear_manual_override(void) {
    
    // Clear override mode flag
    manualOverride = 0;

    // Exit manual override mode in the encoder
    encoder_exit_manual_override();

    // Stop push button timer
    pb_stop_timer();
}

void tempest_isr_encoder_at_min_value(void) {
    
    // Set motor state to STOP
    motorState = TEMPEST_MOTOR_STOP; // Update motor state
    tempest_stop_motor();
}

void tempest_isr_encoder_at_max_value(void) {
    motorState = TEMPEST_MOTOR_STOP; // Update motor state
    tempest_stop_motor(); // Stop the motor
}

void tempest_isr_pushbutton_timer(uint8_t pushbutton) {

    switch (pushbutton) {
    case UP_PUSH_BUTTON:
        
        break;
    case DOWN_PUSH_BUTTON:

        break;
    default:
        break;
    }

}