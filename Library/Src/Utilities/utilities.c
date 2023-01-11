/**
 * @file utilities.c
 * @author Gian Barta-Dougall
 * @brief System file for utilities
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "utilities.h"
#include "log.h"
#include "version_config.h"

/* Private STM Includes */

/* Private #defines */

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */
void utilities_hardware_init(void);

/* Public Functions */

void utilities_init(void) {

    // Initialise hardware
    utilities_hardware_init();
}

/* Private Functions */

/**
 * @brief Initialise the hardware for the library.
 */
void utilities_hardware_init(void) {}

void utils_print_gpio_port(GPIO_TypeDef* gpio) {

    // Return if debugging not enabled
#ifndef DEBUG_LOG_MODULE_ENABLED
    return;
#endif

    // Print every register in GPIO
    char message[(32 + 4) * 9];
    sprintf(message,
            "GPIO PORT: %s\r\nMODER: %lx\r\nOTYPER: %lx\r\nOSPEEDR: %lx\r\nPUPDR: %lx\r\nIDR: %lx\r\nODR: %lx\r\nLCKR: "
            "%lx\r\nAFRL: "
            "%lx\r\nAFRH: %lx\r\n",
            (gpio == GPIOA ? "GPIOA" : "GPIOB"), gpio->MODER, gpio->OTYPER, gpio->OSPEEDR, gpio->PUPDR, gpio->IDR,
            gpio->ODR, gpio->LCKR, gpio->AFR[0], gpio->AFR[1]);
    log_prints(message);
}