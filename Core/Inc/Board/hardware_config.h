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
#define TIMER_FREQUENCY_1KHz 1000
#define TIMER_FREQUENCY_1MHz 1000000

// TEMPORARY MARCO: TODO IS MAKE A WHOE CLOCK CONFIG FILE. This currently
// just copies value from STM32 file
#define SYSTEM_CLOCK_CORE 4000000

// Define ports and pins for peripherals that have been enabled in configuration.h file

/********** Marcos for hardware related to the buttons **********/
#define HC_BUTTON_0_PORT GPIOA
#define HC_BUTTON_1_PORT GPIOB

#define HC_BUTTON_0_PIN 1
#define HC_BUTTON_1_PIN 5
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

/********** Marcos for hardware related to the task scheduler **********/

#define HC_TS_TIMER              TIM15
#define HC_TS_TIMER_CLK_ENABLE() __HAL_RCC_TIM15_CLK_ENABLE()
#define HC_TS_TIMER_FREQUENCY    TIMER_FREQUENCY_1KHz
#define HC_TS_TIMER_MAX_COUNT    65535
#define HC_TS_TIMER_IRQn         TIM1_BRK_TIM15_IRQn
#define HC_TS_TIMER_ISR_PRIORITY TIM15_ISR_PRIORITY

/***********************************************************************/

/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void hardware_config_init(void);

#endif // HARDWARE_CONFIG_H
