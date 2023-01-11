/**
 * @file testing.c
 * @author Gian Barta-Dougall
 * @brief System file for testing
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "testing.h"

#include "utilities.h"
#include "task_scheduler_1.h"
#include "adc_config.h"
#include "hardware_config.h"
#include "ambient_light_sensor.h"
#include "button.h"
#include "encoder.h"
#include "piezo_buzzer.h"
#include "synchronous_interrupts.h"
#include "synchronous_timer.h"
#include "led.h"
#include "log.h"

/* Private STM Includes */

/* Private #defines */

/* Private Structures and Enumerations */
struct Task1 printTimerCount1 = {
    .delay      = 500,
    .functionId = 0,
    .group      = TEMPEST_GROUP,
    .nextTask   = &printTimerCount1,
};

/* Private Variable Declarations */

extern uint32_t tempestTasksFlag;

/* Private Function Prototypes */
void testing_hardware_init(void);

/* Public Functions */

extern uint32_t tempestTasksFlag;
extern uint32_t buttonTasksFlag;

void task0(void) {
    log_prints("Pressed\r\n");
}

void task1(void) {
    log_prints("Released\r\n");
}

void task2(void) {
    log_prints("Single click\r\n");
}

void task3(void) {
    log_prints("Double click\r\n");
}

void task4(void) {
    log_prints("Press and hold\r\n");
}

void button_test(void) {

    hardware_config_init();
    ts_init();
    log_clear();
    button_init();

    // GPIOA->MODER &= ~(0x03 << 12);
    // GPIOA->MODER |= (0x01 << 12);

    while (1) {
        ts_process_internal_flags();
        button_process_internal_flags();
    }
}

void adc_test(void) {

    adc_config_als1_init();

    char m[40];

    while (1) {

        sprintf(m, "Voltage = %i\r\n", adc_config_adc1_convert());
        log_prints(m);
        adc_config_adc1_disable();
        log_prints("HEre\r\n");
        HAL_Delay(500);
    }
}

void als_test(void) {
    hardware_config_init();
    log_clear();
    ts_init();

    uint8_t status1 = 0;
    uint8_t status2 = 0;

    while (1) {

        al_sensor_process_internal_flags();
        ts_process_internal_flags();

        if (status1 != al_sensor_light_found(AL_SENSOR_1_ID)) {
            status1 = al_sensor_light_found(AL_SENSOR_1_ID);
            switch (al_sensor_light_found(AL_SENSOR_1_ID)) {
                case 0:
                    log_prints("SENSOR 1 = DARK\r\n");
                    break;
                case 1:
                    log_prints("SENSOR 1 = LIGHT\r\n");
                    break;
                default:
                    log_prints("SENSOR 1 DISCONNECTED\r\n");
                    break;
            }
        }

        if (status2 != al_sensor_light_found(AL_SENSOR_2_ID)) {
            status2 = al_sensor_light_found(AL_SENSOR_2_ID);
            switch (al_sensor_light_found(AL_SENSOR_2_ID)) {
                case 0:
                    log_prints("SENSOR 2 = DARK\r\n");
                    break;
                case 1:
                    log_prints("SENSOR 2 = LIGHT\r\n");
                    break;
                default:
                    log_prints("SENSOR 2 DISCONNECTED\r\n");
                    break;
            }
        }
    }
}

void piezo_buzzer_test(void) {

    hardware_config_init();

    while (1) {
        piezo_buzzer_play_sound(SOUND);
        // char m[60];
        // sprintf(m, "CNT: %li\r\n", TIM16->CNT);
        // log_prints(m);

        HAL_Delay(3000);
    }
}

void al_sensor_test(void) {

    hardware_config_init();
    synchronous_timer_enable();
    log_clear();

    log_prints("Initialised\r\n");

    while (1) {
        HAL_Delay(1000);

        al_sensor_process_internal_flags();

        if (al_sensor_status(AL_SENSOR_1_ID) == CONNECTED) {
            led_on(LED_ORANGE_ID);
        } else {
            led_off(LED_ORANGE_ID);
        }
    }
}

void encoder_test(void) {

    hardware_config_init();
    uint32_t currentCount = 0;
    log_clear();
    log_prints("Ready.\r\n");
    encoder_init();

    while (1) {

        // Print the encoder count if it changes
        if (currentCount != encoder_get_count(ENCODER_1_ID)) {
            char m[60];
            sprintf(m, "Enocder Count: %li\r\n", encoder_get_count(ENCODER_1_ID));
            log_prints(m);
            currentCount = encoder_get_count(ENCODER_1_ID);
        }
    }
}

void motor_test(void) {

    hardware_config_init();
    log_clear();
    encoder_init();
    ts_init();
    ts_add_task_to_queue(&printTimerCount1);

    SET_PIN_MODE_INPUT(GPIOA, 9);
    SET_PIN_MODE_INPUT(GPIOA, 10);
    SET_PIN_MODE_OUTPUT(GPIOA, 9);
    SET_PIN_MODE_OUTPUT(GPIOA, 10);

    while (1) {

        if (FLAG_IS_SET(tempestTasksFlag, 0)) {
            char m[60];
            sprintf(m, "CNT: %li\r\n", TIM1->CNT);
            log_prints(m);
        }
    }
}

void uart_test(void) {

    hardware_config_init();

    log_clear();

    while (1) {

        // char c = log_getc();
        // // sprintf(msg, "%x\r\n", c);
        // // log_prints(msg);
        // if (c == 0x0D) {
        //     msg[i++] = '\r';
        //     msg[i++] = '\n';
        //     msg[i++] = '\0';
        //     log_prints(msg);
        //     i = 0;
        // } else {
        //     msg[i] = c;
        //     i++;
        // }
    }
}

void testing_init(void) {

    uart_test();
    // Set the GPIO pin to input and read
    while (1) {}
}