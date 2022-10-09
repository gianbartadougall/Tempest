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
#include "task_scheduler.h"
#include "board.h"
#include "motor.h"
#include "hardware_config.h"
#include "adc_config.h"
#include "ambient_light_sensor.h"
/* Private STM Includes */

/* Private #defines */

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */
void testing_hardware_init(void);

/* Public Functions */

extern uint32_t tempestTasksFlag;

Recipe r = {
    .delays      = {1000, 2000, 3000},
    .functionIds = {0, 1, 2},
    .group       = TEMPEST_GROUP,
    .id          = 0,
    .numTasks    = 3,
};

void task0(void) {
    char m[100];
    sprintf(m, "Task 0 executed @t=%li\r\n", TIM15->CNT);
    debug_prints(m);
}

void task1(void) {
    char m[100];
    sprintf(m, "Task 1 executed @t=%li\r\n", TIM15->CNT);
    debug_prints(m);
}

void task2(void) {
    char m[100];
    sprintf(m, "Task 2 executed @t=%li\r\n", TIM15->CNT);
    debug_prints(m);
}

void task3(void) {
    char m[100];
    sprintf(m, "Task 3 executed @t=%li\r\n", TIM15->CNT);
    debug_prints(m);
}

void task4(void) {
    char m[100];
    sprintf(m, "Task 4 executed @t=%li\r\n", TIM15->CNT);
    debug_prints(m);
}

void task5(void) {
    char m[100];
    sprintf(m, "Task 5 executed @t=%li\r\n", TIM15->CNT);
    debug_prints(m);
}

void task6(void) {
    char m[100];
    sprintf(m, "Task 6 executed @t=%li\r\n", TIM15->CNT);
    debug_prints(m);
}

void run_tasks(void) {

    if ((tempestTasksFlag & (0x01 << 0)) != 0) {
        task0();
        tempestTasksFlag &= ~(0x01 << 0);
    }

    if ((tempestTasksFlag & (0x01 << 1)) != 0) {
        task1();
        tempestTasksFlag &= ~(0x01 << 1);
    }

    if ((tempestTasksFlag & (0x01 << 2)) != 0) {
        task2();
        tempestTasksFlag &= ~(0x01 << 2);
    }

    if ((tempestTasksFlag & (0x01 << 3)) != 0) {
        task3();
        tempestTasksFlag &= ~(0x01 << 3);
    }

    if (tempestTasksFlag & (0x01 << 4)) {
        task4();
        tempestTasksFlag &= ~(0x01 << 4);
    }

    if (tempestTasksFlag & (0x01 << 5)) {
        task5();
        tempestTasksFlag &= ~(0x01 << 5);
    }

    if (tempestTasksFlag & (0x01 << 6)) {
        task6();
        tempestTasksFlag &= ~(0x01 << 6);
    }
}

void motor_test() {
    while (1) {
        brd_led_toggle();
        HAL_Delay(500);
        debug_prints("Motor forward!\r\n");
        motor_forward(ROLLER_BLIND_MOTOR_1);
        HAL_Delay(500);
        debug_prints("Motor stop!\r\n");
        motor_stop(ROLLER_BLIND_MOTOR_1);
        HAL_Delay(500);
        debug_prints("Motor reverse!\r\n");
        motor_reverse(ROLLER_BLIND_MOTOR_1);
        HAL_Delay(500);
        debug_prints("Motor brake!\r\n");
        motor_brake(ROLLER_BLIND_MOTOR_1);
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

    uint8_t status = 0;
    while (1) {

        al_sensor_process_flags();

        if (status != al_sensor_status(AL_SENSOR_1)) {
            status = al_sensor_status(AL_SENSOR_1);

            if (status == 1) {
                debug_prints("Light!\r\n");
            } else if (status == 0) {
                debug_prints("Dark\r\n");
            } else if (status == 254) {
                debug_prints("Disconnected\r\n");
            }
        }

        HAL_Delay(1000);

        if (HC_TS_TIMER->CNT > 50000) {
            char m[40];
            sprintf(m, "TIMER CNT: %li\r\n", HC_TS_TIMER->CNT);
            debug_prints(m);
        }
    }
}

void testing_init(void) {

    // Initialise hardware for tests
    hardware_config_init();
    debug_clear();
    ts_init();
    ts_enable_scheduler();

    als_test();

    // Set the GPIO pin to input and read
    while (1) {}
}