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
#include "log.h"

/* Public STM Includes */
#include "stm32l4xx.h"

/* Public Macros */

#define LOG_ERROR(message)                                                                                  \
    do {                                                                                                    \
        char msg[100];                                                                                      \
        sprintf(msg, "System Error File %s, Line number %d.\r\n\t\'%s\'\r\n", __FILE__, __LINE__, message); \
        debug_prints(msg);                                                                                  \
        \                                                                                                   \
    } while (0)

// The definitions for high and low need to be 1 and 0
// because it they are used in if statement calculations
// when checking the states of pins which are defined by
// hardware to be 1 for high and 0 for low. Changing these
// will break any code that uses these defines
#define PIN_LOW  0
#define PIN_HIGH 1

// The definitions for true and false need to be 1 and 0
// because it is convention for this to be so. Changing
// the values to anything else would lead to bugs in a
// program as it is natural to assume false = 0, true = 1
#define FALSE 0
#define TRUE  1

// The values for following #defines are arbitrary and can
// be changed to anything provided they can fit in an 8 bit
// number. The reason they are random values is because it
// helps catch bugs easily because each #define has a unique
// value so accidently mixing macros up becomes obvious
#define CONNECTED    93
#define DISCONNECTED 92

// Used to let calling function know the ID they passed was invalid
// This number is arbitrary however if set to be unique can help
// find any bugs quicker
#define INVALID_ID 94

// Defines for the different configuration modes for the GPIO pins
#define SET_PIN_MODE_INPUT(port, pin)              (port->MODER &= ~(0x03 << (pin * 2)))
#define SET_PIN_MODE_OUTPUT(port, pin)             (port->MODER |= (0x01 << (pin * 2)))
#define SET_PIN_MODE_ALTERNATE_FUNCTION(port, pin) (port->MODER |= (0x02 << (pin * 2)))
#define SET_PIN_MODE_ANALOGUE(port, pin)           (port->MODER |= (0x03 << (pin * 2)))
#define SET_PIN_TYPE_PUSH_PULL(port, pin)          (port->OTYPER &= ~(0x03 << pin))
#define SET_PIN_TYPE_OPEN_DRAIN(port, pin)         (port->OTYPER |= (0x01 << pin))
#define SET_PIN_SPEED_LOW(port, pin)               (port->OSPEEDR &= ~(0x03 << (pin * 2)))
#define SET_PIN_SPEED_MEDIUM(port, pin)            (port->OSPEEDR |= (0x01 << (pin * 2)))
#define SET_PIN_SPEED_HIGH(port, pin)              (port->OSPEEDR |= (0x02 << (pin * 2)))
#define SET_PIN_SPEED_VERY_HIGH(port, pin)         (port->OSPEEDR |= (0x03 << (pin * 2)))
#define SET_PIN_PULL_AS_NONE(port, pin)            (port->PUPDR &= ~(0x03 << (pin * 2)))
#define SET_PIN_PULL_AS_PULL_UP(port, pin)         (port->PUPDR |= (0x01 << (pin * 2)))
#define SET_PIN_PULL_AS_PULL_DOWN(port, pin)       (port->PUPDR |= (0x02 << (pin * 2)))

#define SET_TIMER_DIRECTION_COUNT_UP(timer)   (timer->CR1 &= ~(0x01 << 4))
#define SET_TIMER_DIRECTION_COUNT_DOWN(timer) (timer->CR1 |= (0x01 << 4))

#define SET_PIN_HIGH(port, pin) (port->BSRR |= (0x01 << pin))
#define SET_PIN_LOW(port, pin)  (port->BSRR |= (0x10000 << pin))

#define PIN_IDR_IS_HIGH(port, pin) ((port->IDR & (0x01 << pin)) != 0)
#define PIN_IDR_IS_LOW(port, pin)  ((port->IDR & (0x01 << pin)) == 0)
#define PIN_IDR_STATE(port, pin)   (((port->IDR & (0x01 << pin)) == 0) ? PIN_LOW : PIN_HIGH)

#define PIN_ODR_IS_HIGH(port, pin) ((port->ODR & (0x01 << pin)) != 0)
#define PIN_ODR_IS_LOW(port, pin)  ((port->ODR & (0x01 << pin)) == 0)
#define PIN_ODR_STATE(port, pin)   (((port->ODR & (0x01 << pin)) == 0) ? PIN_LOW : PIN_HIGH)

#define FLAG_IS_SET(flag, bit) ((flag & (0x01 << bit)) != 0)
#define FLAG_CLEAR(flag, bit)  (flag &= ~(0x01 << bit))
#define FLAG_SET(flag, bit)    (flag |= (0x01 << bit))

#define UINT_8_BIT_MAX_VALUE  255
#define UINT_16_BIT_MAX_VALUE 65535
#define UINT_32_BIT_MAX_VALUE 4294967295

/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void utilities_init(void);

void utils_print_gpio_port(GPIO_TypeDef* gpio);

#endif // UTILITIES_H
