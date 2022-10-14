/**
 * @file hardware_configuration.h
 * @author Gian Barta-Dougall
 * @brief This file contains all the pin and GPIO assignments for all the harware peripherals on the board
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

/* Public Includes */
#include "version_config.h"
#include "interrupts_config.h"

/* Public STM Includes */
#include "stm32l4xx_hal.h"

/* Public #defines */

#define LED_ORANGE_PIN  4
#define LED_ORANGE_PORT GPIOB
#define LED_RED_PIN     11
#define LED_RED_PORT    GPIOA

/* Timer Macros */
#define TIMER_FREQUENCY_1KHz   1000
#define TIMER_FREQUENCY_10KHz  10000
#define TIMER_FREQUENCY_100KHz 100000
#define TIMER_FREQUENCY_1MHz   1000000

#define __16_BIT_MAX_COUNT 65535
#define __32_BIT_MAX_COUNT 4294967295

// TEMPORARY MARCO: TODO IS MAKE A WHOE CLOCK CONFIG FILE. This currently
// just copies value from STM32 file
#define SYSTEM_CLOCK_CORE 4000000

// Define ports and pins for peripherals that have been enabled in configuration.h file

/********** Marcos for hardware related to the buttons **********/
#define HC_BUTTON_1_PORT GPIOA
#define HC_BUTTON_2_PORT GPIOB

#define HC_BUTTON_1_PIN 1
#define HC_BUTTON_2_PIN 5

#define HC_BUTTON_1_IQRn EXTI1_IRQn
#define HC_BUTTON_2_IQRn EXTI9_5_IRQn

#define HC_BUTTON_1_ISR_PRIORITY EXTI1_ISR_PRIORITY
#define HC_BUTTON_2_ISR_PRIORITY EXTI9_5_ISR_PRIORITY

#define HC_BUTTON_1_ISR_SUBPRIORITY 0
#define HC_BUTTON_2_ISR_SUBPRIORITY 0
/****************************************************************/

/********** Marcos for hardware related to the motors **********/
#define HC_MOTOR_PORT_1 GPIOA
#define HC_MOTOR_PORT_2 GPIOA
#define HC_MOTOR_PORT_3 GPIOA
#define HC_MOTOR_PORT_4 GPIOB

#define HC_MOTOR_PIN_1 9
#define HC_MOTOR_PIN_2 10
#define HC_MOTOR_PIN_3 12
#define HC_MOTOR_PIN_4 1
/**************************************************************/

/********** Marcos for hardware related to the ambient light sensor **********/
#define HC_ALS_PORT_1 GPIOA
#define HC_ALS_PORT_2 GPIOA

#define HC_ALS_PIN_1 5
#define HC_ALS_PIN_2 4
/*****************************************************************************/

/********** Marcos for hardware related to the LEDs **********/
#define HC_LED_RED_PORT    GPIOA
#define HC_LED_GREEN_PORT  GPIOB
#define HC_LED_ORANGE_PORT GPIOB

#define HC_LED_RED_PIN    11
#define HC_LED_GREEN_PIN  3
#define HC_LED_ORANGE_PIN 4
/*************************************************************/

/********** Marcos for hardware related to the task scheduler **********/
/**
 * The task scheduler schedules tasks that needs run both at specific times
 * but asycnhonously. An example is some task that needs to run t time units
 * after a given event occurs. The task scheduler will ensure that the task
 * is run at the appropriate time
 */
#define HC_TS_TIMER              TIM15
#define HC_TS_TIMER_CLK_ENABLE() __HAL_RCC_TIM15_CLK_ENABLE()
#define HC_TS_TIMER_FREQUENCY    TIMER_FREQUENCY_1KHz
#define HC_TS_TIMER_MAX_COUNT    __16_BIT_MAX_COUNT
#define HC_TS_TIMER_IRQn         TIM1_BRK_TIM15_IRQn
#define HC_TS_TIMER_ISR_PRIORITY TIM15_ISR_PRIORITY
/***********************************************************************/

/********** Marcos for hardware related to the Synchronous timer **********/
/**
 * The synchronous timer generates an interrupt every t time units. In the
 * interrupt, synchronous tasks that need to happen can be run
 */
#define HC_SYNCH_TIMER              TIM3
#define HC_SYNCH_TIMER_CLK_ENABLE() __HAL_RCC_TIM3_CLK_ENABLE()
#define HC_SYNCH_TIMER_FREQUENCY    TIMER_FREQUENCY_1MHz
#define HC_SYNCH_TIMER_MAX_COUNT    1000 // Frequency of 1Mhz will give a 1ms timer if max count is 1000
#define HC_SYNCH_TIMER_IRQn
#define HC_SYNCH_TIMER_ISR_PRIORITY TIM3_ISR_PRIORITY
/***********************************************************************/

/********** Marcos for hardware related to the Piezo Buzzer **********/
/**
 * The pizeo buzzer timer used for generated PWM signals to make sounds
 * on the piezo buzzer
 */
#define HC_PIEZO_BUZZER_PORT GPIOA
#define HC_PIEZO_BUZZER_PIN  6

#define HC_PIEZO_BUZZER_TIMER              TIM16
#define HC_PIEZO_BUZZER_TIMER_CLK_ENABLE() __HAL_RCC_TIM16_CLK_ENABLE()
#define HC_PIEZO_BUZZER_TIMER_FREQUENCY    TIMER_FREQUENCY_100KHz
#define HC_PIEZO_BUZZER_TIMER_MAX_COUNT    __16_BIT_MAX_COUNT
#define HC_PIEZO_BUZZER_TIMER_IRQn         TIM1_UP_TIM16_IRQn
#define HC_PIEZO_BUZZER_TIMER_ISR_PRIORITY TIM1_UP_TIM16_ISR_PRIORITY
/***********************************************************************/

/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void hardware_config_init(void);

#endif // HARDWARE_CONFIG_H
