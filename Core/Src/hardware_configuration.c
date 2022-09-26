/**
 * @file hardware_configuration.c
 * @author Gian Barta-Dougall
 * @brief System file for hardware_configuration
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "hardware_configuration.h"
#include "tempest_configuration.h"
#include "stm32l4xx_hal.h"

/* Private STM Includes */

/* Private #defines */

// Defines for the different configuration modes for the GPIO pins
#define MODER_INPUT                0x00
#define MODER_ALTERNATE_FUNCTION   0x01
#define MODER_OUTPUT               0x02
#define MODER_ANALOGUE             0x03
#define OTYPER_PUSH_PULL           0x00
#define OTYPER_OPEN_DRAIN          0x01
#define OSPEER_LOW_SPEED           0x00
#define OSPEER_MEDIUM_SPEED        0x01
#define OSPEER_HIGH_SPEED          0x02
#define OSPEER_VERY_HIGH_SPEED     0x03
#define PUPDR_NO_PULL_UP_PULL_DOWN 0x00
#define PUPDR_PULL_UP              0x01
#define PUPDR_PULL_DOWN            0x02

// Define ports and pins for peripherals that have been enabled in configuration.h file
#ifdef BUTTON_MODULE_ENABLED
    // Define GPIO port and pin for pushbutton 0
    #define HC_BUTTON_0_PORT GPIOA
    #define HC_BUTTON_0_PIN  1

    // Define GPIO port and pin for pushbutton 1
    #define HC_BUTTON_1_PORT GPIOB
    #define HC_BUTTON_1_PIN  5
#endif

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */
void hardware_config_hardware_init(void);
void hardware_configuration_gpio_init(void);

/* Public Functions */

void hardware_config_init(void) {

    // Initialise GPIO pins
    hardware_configuration_gpio_init();
}

/* Private Functions */

/**
 * @brief Initialise the GPIO pins used by the system.
 */
void hardware_configuration_gpio_init(void) {

    // Set all GPIO configurations to 0 (input, pushpull, low speed, no pull up or down resistor)
    GPIOA->MODER   = 0x00;
    GPIOA->OTYPER  = 0x00;
    GPIOA->OSPEEDR = 0x00;
    GPIOA->PUPDR   = 0x00;
    GPIOB->MODER   = 0x00;
    GPIOB->OTYPER  = 0x00;
    GPIOB->OSPEEDR = 0x00;
    GPIOB->PUPDR   = 0x00;

    // Configure GPIO pins for the pushbuttons if the module is enabled for the current version
    // being compiled
#ifdef PUSHBUTTON_MODULE_ENABLED

    // Configure both push button GPIO pins to be outputs
    HC_BUTTON_0_PORT->MODER |= (MODER_OUTPUT << HC_BUTTON_0_PIN);
    HC_BUTTON_1_PORT->MODER |= (MODER_OUTPUT << HC_BUTTON_1_PIN);

    // Configure both push button GPIO pins to be pushpull
    HC_BUTTON_0_PORT->OTYPER |= (OTYPER_PUSH_PULL << HC_BUTTON_0_PIN);
    HC_BUTTON_1_PORT->OTYPER |= (OUTPUT_PUSH_PULL << HC_BUTTON_1_PIN);

#endif
}