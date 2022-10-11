/**
 * @file utilities.h
 * @author Gian Barta-Dougall
 * @brief System file for utilities
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef UTILITIES_H
#define UTILITIES_H

/* Public Includes */
#include "debug_log.h"

/* Public STM Includes */
#include "stm32l4xx.h"

/* Public #defines */
#define PIN_LOW  0
#define PIN_HIGH 1

#define TRUE  1
#define FALSE 0

#define SET_PIN_MODE_INPUT(port, pin)    (port->MODER &= ~(0x03 << (pin * 2)))
#define SET_PIN_MODE_OUTPUT(port, pin)   (port->MODER |= (0x01 << (pin * 2)))
#define SET_PIN_MODE_ANALOGUE(port, pin) (port->MODER |= (0x03 << (pin * 2)))

#define SET_PIN_HIGH(port, pin) (port->BSRR |= (0x01 << pin))
#define SET_PIN_LOW(port, pin)  (port->BSRR |= (0x10000 << pin))

#define PIN_IS_HIGH(port, pin) ((port->IDR & (0x01 << pin)) != 0)
#define PIN_IS_LOW(port, pin)  ((port->IDR & (0x01 << pin)) == 0)
/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void utilities_init(void);

#endif // UTILITIES_H
