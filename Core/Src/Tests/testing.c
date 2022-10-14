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
#include "board.h"
#include "adc_config.h"
#include "hardware_config.h"
#include "ambient_light_sensor.h"
#include "button.h"
#include "piezo_buzzer.h"

/* Private STM Includes */

/* Private #defines */

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */
void testing_hardware_init(void);

/* Public Functions */

extern uint32_t tempestTasksFlag;
extern uint32_t buttonTasksFlag;

void task0(void) {
    debug_prints("Pressed\r\n");
}

void task1(void) {
    debug_prints("Released\r\n");
}

void task2(void) {
    debug_prints("Single click\r\n");
}

void task3(void) {
    debug_prints("Double click\r\n");
}

void task4(void) {
    debug_prints("Press and hold\r\n");
}

void button_test(void) {

    hardware_config_init();
    ts_init();
    debug_clear();
    button_init();

    // GPIOA->MODER &= ~(0x03 << 12);
    // GPIOA->MODER |= (0x01 << 12);

    while (1) {
        ts_process_flags();
        button_process_flags();
    }
}

void adc_test(void) {

    adc_config_als1_init();

    char m[40];

    while (1) {

        sprintf(m, "Voltage = %i\r\n", adc_config_adc1_convert());
        debug_prints(m);
        adc_config_adc1_disable();
        debug_prints("HEre\r\n");
        HAL_Delay(500);
    }
}

void als_test(void) {
    hardware_config_init();
    debug_clear();
    ts_init();

    uint8_t status1 = 0;
    uint8_t status2 = 0;

    while (1) {

        al_sensor_process_flags();
        ts_process_flags();

        if (status1 != al_sensor_read_status(AL_SENSOR_1)) {
            status1 = al_sensor_read_status(AL_SENSOR_1);
            switch (al_sensor_read_status(AL_SENSOR_1)) {
                case 0:
                    debug_prints("SENSOR 1 = DARK\r\n");
                    break;
                case 1:
                    debug_prints("SENSOR 1 = LIGHT\r\n");
                    break;
                default:
                    debug_prints("SENSOR 1 DISCONNECTED\r\n");
                    break;
            }
        }

        if (status2 != al_sensor_read_status(AL_SENSOR_2)) {
            status2 = al_sensor_read_status(AL_SENSOR_2);
            switch (al_sensor_read_status(AL_SENSOR_2)) {
                case 0:
                    debug_prints("SENSOR 2 = DARK\r\n");
                    break;
                case 1:
                    debug_prints("SENSOR 2 = LIGHT\r\n");
                    break;
                default:
                    debug_prints("SENSOR 2 DISCONNECTED\r\n");
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
        // debug_prints(m);

        HAL_Delay(3000);
    }
}

void testing_init(void) {

    // Initialise hardware for tests
    piezo_buzzer_test();
    // Set the GPIO pin to input and read
    while (1) {}
}