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
#include "utilities.h"

/* Public STM Includes */
#include "stm32l4xx_hal.h"

/* Timer Macros */
#define TIMER_FREQUENCY_1KHz   1000
#define TIMER_FREQUENCY_10KHz  10000
#define TIMER_FREQUENCY_100KHz 100000
#define TIMER_FREQUENCY_1MHz   1000000

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
#define HC_TS_TIMER_MAX_COUNT    UINT_16_BIT_MAX_VALUE
#define HC_TS_TIMER_IRQn         TIM1_BRK_TIM15_IRQn
#define HC_TS_TIMER_ISR_PRIORITY TIM15_ISR_PRIORITY
/***********************************************************************/

/********** Marcos for hardware related to the Synchronous timer **********/
/**
 * The synchronous timer generates an interrupt every t time units. In the
 * interrupt, synchronous tasks that need to happen can be run
 */
#define HC_SYNCH_TIMER              TIM6
#define HC_SYNCH_TIMER_CLK_ENABLE() __HAL_RCC_TIM6_CLK_ENABLE()
#define HC_SYNCH_TIMER_FREQUENCY    TIMER_FREQUENCY_1MHz
#define HC_SYNCH_TIMER_MAX_COUNT    10000 // Frequency of 1Mhz will give a 10ms timer if max count is 10000
#define HC_SYNCH_TIMER_IRQn         TIM6_IRQn
#define HC_SYNCH_TIMER_ISR_PRIORITY TIM6_ISR_PRIORITY
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
#define HC_PIEZO_BUZZER_TIMER_MAX_COUNT    UINT_16_BIT_MAX_VALUE
#define HC_PIEZO_BUZZER_TIMER_IRQn         TIM1_UP_TIM16_IRQn
#define HC_PIEZO_BUZZER_TIMER_ISR_PRIORITY TIM1_UP_TIM16_ISR_PRIORITY
/***********************************************************************/

/********** Marcos for hardware related to the Encoders **********/
/**
 * The timers are used to increment the encoder counts every time the pins
 * they are connected to go high
 */
#define HC_ENCODER_1_PORT         GPIOA
#define HC_ENCODER_1_PIN          8
#define HC_ENCODER_1_PORT_CLK_POS (0x01 << RCC_AHB2ENR_GPIOAEN)
#define HC_ENCODER_1_IRQn         EXTI9_5_IRQn

#define HC_ENCODER_1_TIMER              TIM1
#define HC_ENCODER_1_TIMER_CLK_ENABLE() __HAL_RCC_TIM1_CLK_ENABLE()
#define HC_ENCODER_1_TIMER_FREQUENCY    TIMER_FREQUENCY_1KHz
#define HC_ENCODER_1_TIMER_MAX_COUNT    UINT_32_BIT_MAX_VALUE
#define HC_ENCODER_1_TIMER_IRQn         TIM1_CC_IRQn
#define HC_ENCODER_1_TIMER_ISR_PRIORITY TIM1_ISR_PRIORITY

#define HC_ENCODER_2_PORT         GPIOA
#define HC_ENCODER_2_PIN          0
#define HC_ENCODER_PORT_2_CLK_POS (0x01 << RCC_AHB2ENR_GPIOAEN)
#define HC_ENCODER_2_IRQn         EXTI0_IRQn

#define HC_ENCODER_2_TIMER              TIM2
#define HC_ENCODER_2_TIMER_CLK_ENABLE() __HAL_RCC_TIM2_CLK_ENABLE()
#define HC_ENCODER_2_TIMER_FREQUENCY    TIMER_FREQUENCY_1KHz
#define HC_ENCODER_2_TIMER_MAX_COUNT    UINT_32_BIT_MAX_VALUE
#define HC_ENCODER_2_TIMER_IRQn         TIM2_IRQn
#define HC_ENCODER_2_TIMER_ISR_PRIORITY TIM2_ISR_PRIORITY
/***********************************************************************/

/********** Marcos for hardware related to the debug log **********/
/**
 * Configuration for UART which allows debuggiong and general information
 * transfer
 */
#define HC_DEBUG_LOG_GPIO_RX_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define HC_DEBUG_LOG_GPIO_TX_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define HC_DEBUG_LOG_CLK_ENABLE()         __HAL_RCC_USART2_CLK_ENABLE()

#define HC_DEBUG_LOG_RX_PORT   GPIOA
#define HC_DEBUG_LOG_TX_PORT   GPIOA
#define HC_DEBUG_LOG_RX_PIN    3
#define HC_DEBUG_LOG_TX_PIN    2
#define HC_DEBUG_LOG_BUAD_RATE 115200
/**************************************************************/

/**
 * @brief Initialise the system library.
 */
void hardware_config_init(void);

#endif // HARDWARE_CONFIG_H
