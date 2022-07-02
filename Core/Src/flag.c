/**
 * @file flag.h
 * @author Gian Barta-Dougall
 * @brief Peripheral driver for a generic flag
 * @version 0.1
 * @date 2022-07-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/* Public Includes */

/* Private Includes */
#include "flag.h"

/* STM32 Includes */

/* Private #defines */
#define FLAG_0_PIN_RAW 5
#define FLAG_0_PIN (FLAG_0_PIN_RAW * 2)
#define FLAG_0_PORT_CLK_POS (0x01 << RCC_AHB2ENR_GPIOBEN)
#define FLAG_0_IRQn EXTI9_5_IRQn

/* Function prototypes */

void flag_init(void) {

    // Initialise manual override pin
    FLAG_0_PORT->MODER   &= ~(0x03 << FLAG_0_PIN); // Set pin to input mode
    FLAG_0_PORT->OSPEEDR &= ~(0x03 << FLAG_0_PIN); // Set pin to low speed
    FLAG_0_PORT->PUPDR   &= ~(0x03 << FLAG_0_PIN); // Reset pull up pull down pin
    FLAG_0_PORT->PUPDR   |= (0x02 << FLAG_0_PIN); // Set pin to pull down
    FLAG_0_PORT->OTYPER  &= ~(0x01 << FLAG_0_PIN_RAW); // Set pin to push-pull

    // Enable GPIO Clock
    RCC->AHB2ENR |= FLAG_0_PORT_CLK_POS;

    // Configure interrupt for manual override pin
    SYSCFG->EXTICR[1] &= ~(0x07 << (1  * 4)); // Clear trigger line
    SYSCFG->EXTICR[1] |= (0x01 << (1  * 4)); // Set line for PB5

    EXTI->RTSR1 |= EXTI_RTSR1_RT5; // Enable trigger on rising edge
    EXTI->FTSR1 |= EXTI_FTSR1_FT5; // Enable interrupt on falling edge
    EXTI->IMR1  |= EXTI_IMR1_IM5; // Enabe external interrupt for EXTI line

    // Configure interrupt priorities
    HAL_NVIC_SetPriority(FLAG_0_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(FLAG_0_IRQn);
}

uint8_t flag_state(uint8_t flag) {
    return ((FLAG_0_PORT->IDR & (0x01 << FLAG_0_PIN_RAW)) != 0) ? 1 : 0;
}