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

// Relating the mode of the system to the different pushbutton modes
#define TEMPEST_MODE_MANUAL PB_MODE_0
#define TEMPEST_MODE_AUTOMATIC PB_MODE_1
#define TEMPEST_MODE_MANUAL_OVERRIDE PB_MODE_2

#define PB_UP PUSH_BUTTON_0
#define PB_DOWN PUSH_BUTTON_1

/* Private #defines */
#define UP_PUSH_BUTTON PUSH_BUTTON_0
#define DOWN_PUSH_BUTTON PUSH_BUTTON_1

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
uint8_t systemMode;
uint8_t motorState = TEMPEST_MOTOR_STOP;
uint32_t modeSwitchStartTime = 0;
uint8_t manualOverride = 0;
uint8_t manualOverrideMoveUpFlag = 0;
uint8_t manualOverrideMoveDownFlag = 0;

uint32_t encoderLastCount = 0;

TimerMsTask ModeSwitchTask; // Task task used to switch modes
TimerMsTask AlsTask; // Task to record ambient light
TimerMsTask PbUpManualOverrideTask;
TimerMsTask PbDownManualOverrideTask;
TimerMsTask EncoderRunningTask;

/* Function prototypes */
void tempest_error_handler(void);
uint8_t tempest_decode_input_state(void);
void tempest_stop_motor(void);
void tempest_set_motor_up(void);
void tempest_set_motor_down(void);
void tempest_init_timer_ms(void);

/**
 * @brief ISR to be called when a mode switch needs to occur. Function sets
 * mode to manual if it's currently automatic and sets mode to automatic if
 * it's currently manual. Updates LED status based on new mode. Plays sound
 * on piezo buzzer to indicate mode switch has taken place
 */
void tempest_mode_switch_isr(void) {

    switch (systemMode) {
        case TEMPEST_MODE_MANUAL:
            systemMode = TEMPEST_MODE_AUTOMATIC;
            piezo_buzzer_play_sound(sound1);
            brd_led_on(); // Update LED
            break;
        case TEMPEST_MODE_AUTOMATIC:
            systemMode = TEMPEST_MODE_MANUAL;
            piezo_buzzer_play_sound(sound1);
            brd_led_off(); // Update LED
            break;
        default:
            break;
    }
}

void tempest_stop_motor(void) {

    debug_prints("Stopping motor\r\n");
    if (motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_STOP) != HAL_OK) {
        tempest_error_handler();
    }

    timer_ms_cancel_task(&EncoderRunningTask);
}

void tempest_set_motor_up(void) {
    
    encoderLastCount = encoder_get_count();
    
    debug_prints("Motor UP\r\n");
    // Set the direction of the encoder
    encoder_set_direction_negative();

    if ((systemMode != TEMPEST_MODE_MANUAL_OVERRIDE) && encoder_at_minimum_distance()) {
        return;
    }
  
    if (motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_UP) != HAL_OK) {
        tempest_error_handler();
    }

    if (systemMode == TEMPEST_MODE_AUTOMATIC) {
        debug_prints("STARTING ENCODER RUNNING TASK UP\r\n");
        timer_ms_add_task(&EncoderRunningTask);
    }
}

void tempest_set_motor_down(void) {
    encoderLastCount = encoder_get_count();

    debug_prints("Motor DOWN\r\n");
    // Set the direction of the encoder
    encoder_set_direction_positive();
    
    if ((systemMode != TEMPEST_MODE_MANUAL_OVERRIDE) && encoder_at_maximum_distance()) {
        return;
    }

    if (motor_driver_set_motor_state(TEMPEST_MOTOR, MOTOR_DOWN) != HAL_OK) {
        tempest_error_handler();
    }

    if (systemMode == TEMPEST_MODE_AUTOMATIC) {
        debug_prints("STARTING ENCODER RUNNING TASK DOWN\r\n");
        timer_ms_add_task(&EncoderRunningTask);
    }
}

void tempest_isr_confirm_encoder_running(void) {
    debug_prints("Confirming encoder is running\r\n");
    // Check whether the last the current encoder count has changed since
    // this function was last called. If it hasn't, then the counter isn't
    // picking up encoder counts thus stopping the motor, disabling everything
    // and entering error handler
    if (encoder_get_count() == encoderLastCount) {
        
        // Stop the motor
        tempest_stop_motor();

        // Disable enocder
        encoder_disable();

        // Disable GPIO clocks to prevent any pins from toggling
        __HAL_RCC_GPIOA_CLK_DISABLE();
        __HAL_RCC_GPIOB_CLK_DISABLE();
        
        // Call error handler
        tempest_error_handler();

    } else {
        encoderLastCount = encoder_get_count();
    }

}

void tempest_init_timer_ms(void) {
 
    timer_ms_init(); // Initialise hardware for millisecond timer
    timer_ms_enable(); // Enable timer
    
    /* Initialise the handles for required delays */
	
	void (*t1ISRs[1]) (void) = {&tempest_mode_switch_isr};
	uint16_t t1Delays[1] = {3000}; // Wait 3 seconds before calling ISR 
	timer_ms_init_task(
		&ModeSwitchTask, t1ISRs, t1Delays, 1, TIMER_MS_TASK_PAUSE, TASK_MS_PRIORITY_3, TIMER_MS_TASK_FINISH
	);

	void (*t2ISRs[3]) (void) = {&ambient_light_sensor_isr_s1, &ambient_light_sensor_isr_s2, &ambient_light_sensor_isr_s3};
	uint16_t t2Delays[3] = {
		60000, // Wait 30 seconds for capacitor to charge before setting pin to input
		1, // Read voltage on capacitor 1ms after pin is set to input
		3000 // Wait 3 seconds for capacitor to discharge
	}; 
	timer_ms_init_task(
		&AlsTask, t2ISRs, t2Delays, 3, TIMER_MS_TASK_CANCEL, TASK_MS_PRIORITY_4, TIMER_MS_TASK_FINISH
	);

    void (*t3ISRs[1]) (void) = {&tempest_set_motor_up};
    uint16_t t3Delays[1] = {3000};
    timer_ms_init_task(
        &PbUpManualOverrideTask, t3ISRs, t3Delays, 1, TIMER_MS_TASK_PAUSE, TASK_MS_PRIORITY_3, TIMER_MS_TASK_FINISH
    );

    void (*t4ISRs[1]) (void) = {&tempest_set_motor_down};
    uint16_t t4Delays[1] = {3000};
    timer_ms_init_task(
        &PbDownManualOverrideTask, t4ISRs, t4Delays, 1, TIMER_MS_TASK_PAUSE, TASK_MS_PRIORITY_3, TIMER_MS_TASK_FINISH
    );

    void (*t5ISRs[1]) (void) = {&tempest_isr_confirm_encoder_running};
    uint16_t t5Delays[1] = {1000};
    timer_ms_init_task(
        &EncoderRunningTask, t5ISRs, t5Delays, 1, TIMER_MS_TASK_PAUSE, TASK_MS_PRIORITY_1, TIMER_MS_TASK_REPEAT
    );

}

void tempest_error_handler(void) {

    while (1) {
        piezo_buzzer_play_sound(sound1);
        HAL_Delay(10000);
    }
}

void play_buzzer(uint8_t length) {
    piezo_buzzer_play_sound(sound2);
    HAL_Delay(length);
    piezo_buzzer_play_sound(sound2);
    HAL_Delay(length);
    piezo_buzzer_play_sound(sound2);
}

void tempest_isr_encoder_at_min_value(void) {

    // If the system is in manual override, ignore ISR to stop motor
    if (systemMode != TEMPEST_MODE_MANUAL_OVERRIDE) {
        tempest_stop_motor(); // Stop the motor
    }
}

void tempest_isr_encoder_at_max_value(void) {

    if (systemMode != TEMPEST_MODE_MANUAL_OVERRIDE) {
        tempest_stop_motor(); // Stop the motor
    }
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


/* Configure callbacks for pushbuttons */

/**
 * @brief Determines what happens when the UP pushbutton is pressed whilst
 * the system is in manual override 
 */
void pb_up_re_manual_override(void) {
    timer_ms_add_task(&PbUpManualOverrideTask); // Start manual override task
}

/**
 * @brief Determines what happens when the UP pushbutton is pressed whilst
 * the system is in manual override 
 */
void pb_up_fe_manual_override(void) {
    // Stop the motor
    tempest_stop_motor();

    // If the task is still in the queue then it never finished thus set the
    // new maximum value for blind
    if (timer_ms_task_is_in_queue(&PbUpManualOverrideTask) == 1) {
        
        // Cancel task
        timer_ms_cancel_task(&PbUpManualOverrideTask);
        
        // Set new max value
        encoder_isr_reset_min_value();
    }
}

/**
 * @brief Determines what happens when the UP pushbutton is pressed whilst
 * the system is in manual override 
 */
void pb_down_re_manual_override(void) {
    timer_ms_add_task(&PbDownManualOverrideTask); // Start manual override task
}

/**
 * @brief Determines what happens when the UP pushbutton is pressed whilst
 * the system is in manual override 
 */
void pb_down_fe_manual_override(void) {
    // Stop the motor
    tempest_stop_motor();

    // If the task is still in the queue then it never finished thus set the
    // new maximum value for blind
    if (timer_ms_task_is_in_queue(&PbDownManualOverrideTask) == 1) {
        
        // Cancel task
        timer_ms_cancel_task(&PbDownManualOverrideTask);
        
        // Set new max value
        encoder_isr_reset_max_value();
    }
}

void empty_func(void) {

}

void tempest_set_mode_manual(void) {

    // Set the system mode to manual
    systemMode = TEMPEST_MODE_MANUAL;

    // Set the pushbutton mode to manual
    pb_set_mode(TEMPEST_MODE_MANUAL);
}

void tempest_set_mode_automatic(void) {

    // Set the system mode to automatic
    systemMode = TEMPEST_MODE_AUTOMATIC;

    // Set the pushbutton mode to automatic
    pb_set_mode(TEMPEST_MODE_AUTOMATIC);
}

void tempest_set_mode_manual_override(void) {
    debug_prints("Entered manual override mode\r\n");
    // Set the system mode to manual override
    systemMode = TEMPEST_MODE_MANUAL_OVERRIDE;

    // Set the pushbutton mode to manual override
    pb_set_mode(TEMPEST_MODE_MANUAL_OVERRIDE);

    // Play sound on buzzer
    play_buzzer(50);
}

void tempest_update_system(void) {

    // Update system state
    if ((pb_get_state(PB_UP) == 1) && (pb_get_state(PB_DOWN) == 1)) {
        
        // Check if the task already exists in the queue
        if (timer_ms_task_is_in_queue(&ModeSwitchTask) == 0) {
            timer_ms_add_task(&ModeSwitchTask);
        }

        return;
    }

    switch (systemMode) {
    case TEMPEST_MODE_MANUAL_OVERRIDE:
        break;

    case TEMPEST_MODE_MANUAL:
        break;
    
    case TEMPEST_MODE_AUTOMATIC:

        // If the als task is not already in the queue, add it to the queue
        if (timer_ms_task_is_in_queue(&AlsTask) == 0) {
            timer_ms_add_task(&AlsTask);
        }

        // Read ambient light sensor value
        uint8_t lightLevel = ambient_light_read();

        // Set the motor to the correct hight
        if ((lightLevel == HIGH) && !encoder_at_minimum_distance()) {
            debug_prints("Setting motor up\r\n");
            tempest_set_motor_up();
        } else if ((lightLevel == LOW) && !encoder_at_maximum_distance()) {
            tempest_set_motor_down();
            debug_prints("Setting motor down\r\n");
        }

        break;
    default:
        break;
    }

    // Cancel task if it exists in the queue
    timer_ms_cancel_task(&ModeSwitchTask);
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
    pb_set_callbacks(PB_UP, TEMPEST_MODE_MANUAL, &tempest_set_motor_up, &tempest_stop_motor);
    pb_set_callbacks(PB_UP, TEMPEST_MODE_AUTOMATIC, &tempest_set_motor_up, &empty_func);
    pb_set_callbacks(PB_UP, TEMPEST_MODE_MANUAL_OVERRIDE, &pb_up_re_manual_override, &pb_up_fe_manual_override);

    pb_set_callbacks(PB_DOWN, TEMPEST_MODE_MANUAL, &tempest_set_motor_down, &tempest_stop_motor);
    pb_set_callbacks(PB_DOWN, TEMPEST_MODE_AUTOMATIC, &tempest_set_motor_down, &empty_func);
    pb_set_callbacks(PB_DOWN, TEMPEST_MODE_MANUAL_OVERRIDE, &pb_down_re_manual_override, &pb_down_fe_manual_override);

    // Initialise generic millisecond timer
    tempest_init_timer_ms();

    // Initialise piezo buzzer
    piezo_buzzer_init();

    // Initialise flag to act as manual override pin
    flag_init();

    // Set system mode to automatic
    tempest_set_mode_automatic();
}