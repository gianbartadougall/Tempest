/**
 * @file hardware_config.h
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

/* Public STM Includes */
#include "stm32l4xx.h"

/* Public #defines */
#define LED_ORANGE_PIN 4
#define LED_ORANGE_PORT GPIOB
#define LED_RED_PIN 11
#define LED_RED_PORT GPIOA

/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void hardware_config_init(void);

#endif // HARDWARE_CONFIG_H
