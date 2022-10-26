/**
 * @file comparator.h
 * @author Gian Barta-Dougall
 * @brief Peripheral driver for the STM32L432KC internal comparators
 * @version 0.1
 * @date 2022-06-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/* Public Includes */

/* Private Includes */
#include "comparator.h"
#include "debug_log.h"

/* STM32 Includes */

/* Private #defines */

// #defines for minus input of comparator
#define COMP1_IN_MINUS_PIN_RAW 4
#define COMP1_IN_MINUS_PIN (COMP1_IN_MINUS_PIN_RAW * 2)
#define COMP1_IN_MINUS_HAL_PIN GPIO_PIN_4
#define COMP1_IN_MINUS_PORT GPIOA

#define COMP1_IN_PLUS_PIN_RAW 1
#define COMP1_IN_PLUS_PIN (COMP1_IN_PLUS_PIN_RAW * 2)
#define COMP1_IN_PLUS_HAL_PIN GPIO_PIN_1
#define COMP1_IN_PLUS_PORT GPIOA


/* Variable Declarations */

/* Function prototypes */
void comparator_hardware_init(void);

void comparator_init(void) {

    comparator_hardware_init();
}

void comparator_hardware_init(void) {

    // Configure GPIO input pin (Minus)
    COMP1_IN_MINUS_PORT->MODER |= (0x03 << COMP1_IN_MINUS_PIN); // Set mode to analogue
    COMP1_IN_MINUS_PORT->PUPDR &= ~(0x03 << COMP1_IN_MINUS_PIN); // Set pupdr to no pull up/down
    COMP1_IN_MINUS_PORT->OTYPER |= (0x01 << COMP1_IN_MINUS_PIN_RAW); // Set type to open drain

    COMP1_IN_PLUS_PORT->MODER |= (0x03 << COMP1_IN_PLUS_PIN); // Set mode to analogue
    COMP1_IN_PLUS_PORT->PUPDR &= ~(0x03 << COMP1_IN_PLUS_PIN); // Set pupdr to no pull up/down
    COMP1_IN_PLUS_PORT->OTYPER |= (0x01 << COMP1_IN_PLUS_PIN_RAW); // Set type to open drain

    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Configure comparator 1
    COMP1->CSR &= ~(0x03 << 25); // Reset input minus pin
    COMP1->CSR |= (0x02 << 25); // Set input minus pin to PA4
    COMP1->CSR |= (0x07 << 4); // Set input for minus pin to be from GPIOx
    
    // Select input plus pin
    COMP1->CSR &= ~(0x03 << 7); // Reset input plus pin
    COMP1->CSR |= (0x02 << 7); // Set input plus pin to PA1
    
    // Add hysterisis to prevent comparator bouncing
    COMP1->CSR &= ~(0x03 << 16); // Reset hysterisis
    COMP1->CSR |= (0x01 << 16); // Set hysterisis to low

    // Enable window mode
    COMP2->CSR |= (0x01 << 9);
    
    // Invert polarity of both comparators
    COMP1->CSR |= (0x01 << 15);
    COMP2->CSR |= (0x01 << 15);

    // Configure EXTI line for comparator 1 interrupts. Comparator 1 output is configured to EXTI 21

    // Enable clock for comparators
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    EXTI->IMR1  |= EXTI_IMR1_IM21; // Enabe external interrupt for EXTI line
    EXTI->RTSR1 |= EXTI_RTSR1_RT21; // Enable trigger on rising edge
    EXTI->FTSR1 |= EXTI_FTSR1_FT21; // Disable interrupt on falling edge

    // Configure interrupt priorities
    HAL_NVIC_SetPriority(COMP_IRQn, 9, 0);
	HAL_NVIC_EnableIRQ(COMP_IRQn);

    // Enable comparator 1 and 2
    COMP1->CSR |= (0x01);
    COMP2->CSR |= (0x01);
}

void window_comparator_hardware_init(void) {



}