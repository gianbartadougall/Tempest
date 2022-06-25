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
#define TEMPEST_BLIND_STATE_UNDEFINED 12

#define TEMPEST_MOTOR MD_MOTOR_0
#define MOTOR_STOP MOTOR_DRIVER_STOP
#define MOTOR_UP MOTOR_DRIVER_DIRECTION_1
#define MOTOR_DOWN MOTOR_DRIVER_DIRECTION_2

#define MIN_MODE_SWITCH_TIME 1000

/* Variable Declarations */
uint8_t mode = TEMPEST_MODE_AUTOMATIC;
uint8_t motorState = MOTOR_STOP;
uint32_t modeSwitchStartTime = 0;

/* Function prototypes */

void tempest_update_system_state(void) {

    // Read the state of the pushbuttons
    uint8_t pbUpState = pb_get_state(UP_PUSH_BUTTON);
    uint8_t pbDownState = pb_get_state(DOWN_PUSH_BUTTON);

    // Update the system state if both pushbuttons are pressed
    if ((pbUpState == 1) && (pbDownState == 1)) {
        tempest_update_mode();
        
        // Exit function
        return;
    } else {
        modeSwitchStartTime = 0; // Reset timer
    }

    // Update system state based on state of pushbuttons if in manual mode
    if (mode == TEMPEST_MODE_MANAUAL) {

        // Set motor to state to MOVE_UP
        if ((pbUpState == 1) && (pbDownState == 0)) {
            motorState = TEMPEST_MOTOR_MOVE_UP;
        }

        if ((pbUpState == 0) && (pbDownState == 1)) {
            motorState = TEMPEST_MOTOR_MOVE_DOWN;
        }

        if ((pbUpState == 0) && (pbDownState == 0)) {
            motorState = TEMPEST_MOTOR_STOP;
        }

    } 
    
    if (mode == TEMPEST_MODE_AUTOMATIC) {

        // Update system state based on state of the ambient light sensor
        uint8_t alsState = ambient_light_sensor_read();

        // Set motor to move up if there is ambient light
        if ((pbUpState == 1) && (pbDownState == 0)) {
            motorState = TEMPEST_MOTOR_MOVE_UP;
        }

        if ((pbUpState == 0) && (pbDownState == 1)) {
            motorState = TEMPEST_MOTOR_MOVE_DOWN;
        }
        // if (alsState == 1) {
        //     motorState = TEMPEST_MOTOR_MOVE_UP;
        // } 
        
        // if (alsState == 0) {
        //     motorState = TEMPEST_MOTOR_MOVE_DOWN;
        // }
    }

}

void tempest_update_motor_state(void) {
    
    if (motorState == TEMPEST_MOTOR_STOP) {
        motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_STOP);
    }

    if (motorState == TEMPEST_MOTOR_MOVE_UP) {
        // Confirm the motor can move up
        if (encoder_at_minimum_distance()) {
            // Stop motor because the blind has reached the minimum distance
            motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_STOP);
        } else {
            motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_UP);
            encoder_set_direction_negative();
        }
    }

    if (motorState == TEMPEST_MOTOR_MOVE_DOWN) {
        // Confirm the motor can move down
        if (encoder_at_maximum_distance()) {
            // Stop motor because the blind has reached the maximum distance
            motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_STOP);
        } else {
            motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_DOWN);
            encoder_set_direction_positive();
        }
    }
}

void tempest_update_mode(void) {

    debug_prints("updating mode\r\n");
    // Turn the motor off
    motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_STOP);

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
        } else if (mode == TEMPEST_MODE_MANAUAL) {
            mode = TEMPEST_MODE_AUTOMATIC;
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