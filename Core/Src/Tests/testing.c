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

/* Private STM Includes */

/* Private #defines */

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */
void testing_hardware_init(void);

/* Public Functions */

extern uint32_t tempestTasksFlag;
extern uint32_t ambientLightSensorFlag;

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

uint16_t adc_convert(void) {

    // ADC1->SQR1 &= ~(0x0F << 6);                 // Reset sequence
    ADC1->SQR1 &= ~(0x0F << 6);                 // Reset sequence
    ADC1->SQR1 &= ~(0x0F << 12);                // Reset sequence
    ADC1->SQR1 &= ~(0x0F << 18);                // Reset sequence
    ADC1->SQR1 &= ~(0x0F << 24);                // Reset sequence
    ADC1->SQR1 |= (0x0A << 6);                  // Set channel 10
    ADC1->SQR1 |= (0x0A << 12);                 // Set channel 10
    ADC1->SQR1 |= (0x0A << 18);                 // Set channel 10
    ADC1->SQR1 |= (0x0A << 24);                 // Set channel 10
    while ((ADC1->CR & ADC_CR_ADSTART) != 0) {} // Ensure there are no converstions currently running

    ADC1->CR |= ADC_CR_ADSTART; // Start regular conversions

    while ((ADC1->ISR & (0x01 << 2)) == 0) {} // Wait for ADC conversion to complete

    return ADC1->DR;
}

void adc_test(void) {

    // Turn the ADC on
    ADC1->CR &= ~(ADC_CR_DEEPPWD); // Take ADC out of deep power down mode
    ADC1->CR |= (ADC_CR_ADVREGEN); // Enable ADC voltage regulator
    HAL_Delay(1);                  // Wait for voltage regulator to stabalise
    ADC1->CR |= (0x01);            // Enable the ADC
    char m[40];

    while ((ADC1->ISR & 0x01) == 0) {} // Wait for ADC to stabalise

    while (1) {

        sprintf(m, "V = %i\r\n", adc_convert());
        debug_prints(m);
        HAL_Delay(500);
    }
}

void testing_init(void) {

    // Initialise hardware for tests
    hardware_config_init();

    adc_test();

    // Set the GPIO pin to input and read
    while (1) {}
}