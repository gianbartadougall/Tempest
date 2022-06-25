/**
 * @file ambient_light_sensor.c
 * @author Gian Barta-Dougall
 * @brief Peripheral driver for a generic ambient light sensor using an integrator circuit
 * @version 0.1
 * @date 2022-06-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/* Public Includes */

/* Private Includes */
#include "ambient_light_sensor.h"

/* STM32 Includes */

/* Private #defines */
#define ALS_PIN_RAW 4
#define ALS_PIN (ALS_PIN_RAW * 2)
#define ALS_PORT GPIOB
#define ALS_PORT_CLK_POS (0x01 << RCC_AHB2ENR_GPIOBEN)
#define ALS_IRQn EXTI4_IRQn

/* Variable Declarations */

/* Function prototypes */
void ambient_light_sensor_hardware_init(void);

void ambient_light_sensor_init(void) {

    // Initialise hardware
    ambient_light_sensor_hardware_init();
}

void ambient_light_sensor_hardware_init(void) {
    
    // Initialise GPIO pin
    ALS_PORT->MODER   &= ~(0x11 << ALS_PIN); // Set pin to input mode (High Z)
    ALS_PORT->OSPEEDR &= ~(0x11 << ALS_PIN); // Set pin to low speed
    ALS_PORT->PUPDR   &= ~(0x11 << ALS_PIN); // Set pin to no pull up/down
    ALS_PORT->OTYPER  |= (0x01 << ALS_PIN_RAW); // Set pin to open drain
}

uint8_t ambient_light_sensor_read(void) {
    return ((ALS_PORT->IDR & (0x01 << ALS_PIN_RAW)) == (0x01 << ALS_PIN_RAW) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}


