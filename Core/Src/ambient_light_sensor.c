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
#include "debug_log.h"
/* STM32 Includes */

/* Private #defines */
#define ALS_PIN_RAW 3
#define ALS_PIN (ALS_PIN_RAW * 2)
#define ALS_PORT GPIOA
#define ALS_PORT_CLK_POS (0x01 << RCC_AHB2ENR_GPIOBEN)
#define ALS_IRQn EXTI5_IRQn

/* Variable Declarations */

/* Function prototypes */
void ambient_light_sensor_hardware_init(void);

void ambient_light_sensor_init(void) {

    // Initialise hardware
    ambient_light_sensor_hardware_init();
}

void ambient_light_sensor_hardware_init(void) {
    
    // Initialise GPIO pin
    ALS_PORT->MODER   &= ~(0x03 << ALS_PIN); // Set pin to analogue mode (High Z)
    ALS_PORT->MODER   |= (0x01 << ALS_PIN);
    ALS_PORT->OSPEEDR &= ~(0x03 << ALS_PIN); // Set pin to low speed
    ALS_PORT->PUPDR   &= ~(0x03 << ALS_PIN); // Set pin to reset pupdr
    ALS_PORT->OTYPER  &= ~(0x01 << ALS_PIN_RAW); // Set pin to push pull

    // Enable clock
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

uint8_t ambient_light_sensor_read(void) {

    // Set pin to input mode
    ALS_PORT->MODER &= ~(0x03 << ALS_PIN); // Set pin to input mode

    // Delay for 1ms to allow IDR to update as it is not update when in analogue mode
    HAL_Delay(1);

    uint8_t value;

    if ((ALS_PORT->IDR & (0x01 << ALS_PIN_RAW)) == (0x01 << ALS_PIN_RAW)) {
        value = GPIO_PIN_SET;
    } else  {
        value = GPIO_PIN_RESET;
    }

    // Set port to output and set low to discharge the capacitor
    ALS_PORT->MODER |= (0x01 << ALS_PIN); // Set pin to output mode
    ALS_PORT->ODR &= ~(0x01 << ALS_PIN_RAW); // Set pin low to discharge capacitor

    // Wait 100ms for capacitor to fully discharge
    HAL_Delay(100);
    
    // Set pin back to high impedance mode
    ALS_PORT->MODER |= (0x03 << ALS_PIN); // Set pin to analogue mode (High Z)

    return value;
}


