/**
 * @file pushbutton.c
 * @author Gian Barta-Dougall
 * @brief Peripheral driver for generic pushbutton
 * @version 0.1
 * @date 2022-06-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/* Public Includes */
#include <stdio.h>

/* Private Includes */
#include "pushbutton.h"
#include "debug_log.h"

/* STM32 Includes */

/* Private #defines */
#define PUSH_BUTTONS 2

/* Variable Declarations */
uint32_t pbPins[PUSH_BUTTONS] = {0, 7};
GPIO_TypeDef* pbPorts[PUSH_BUTTONS] = {GPIOA, GPIOA};
IRQn_Type extiLines[PUSH_BUTTONS] = {EXTI1_IRQn, EXTI9_5_IRQn};

/* Function prototypes */
void pb_hardware_init(uint8_t index);

void pb_init(void) {

    // Initialise pushbutton hardware
    for (int i = 0; i < PUSH_BUTTONS; i++) {

        // pb_hardware_init pin hardware
        pb_hardware_init(i);
        
        // Clear trigger line
        SYSCFG->EXTICR[pbPins[i] / 4] &= ~(0x07 << (4 * pbPins[i])); // Clear trigger line

        // Set trigger line to correct pin. Note PA lines should be set to 0x000 thus the reset above sets
        // the trigger line to PA by default
        if (pbPorts[i] == GPIOB) {
            SYSCFG->EXTICR[pbPins[i] / 4] |= (0x01 << (4 * pbPins[i])); // Set trigger line for given pin
        }

        /* Configure ISR if required */
        
        // EXTI->RTSR1 |= (0x01 << pbPins[i]); // Enable trigger on rising edge
        // EXTI->FTSR1 |= (0x01 << pbPins[i]); // Enable interrupt on falling edge
        // EXTI->IMR1  |= (0x01 << pbPins[i]); // Enabe external interrupt for EXTI line

        // // Enable Clock
        // RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

        // HAL_NVIC_SetPriority(extiLines[i], 10, 0);
        // HAL_NVIC_EnableIRQ(extiLines[i]);
    }
}

void pb_hardware_init(uint8_t index) {

    // Set pushbutton to generic input
    pbPorts[index]->MODER  &= ~(0x03 << (pbPins[index] * 2)); // Set pin to input mode
    pbPorts[index]->OSPEEDR &= ~(0x03 << (pbPins[index] * 2)); // Set pin to low speed
    pbPorts[index]->PUPDR  &= ~(0x03 << (pbPins[index] * 2)); // Set pin to no pull up/down
    pbPorts[index]->OTYPER &= ~(0x01 << pbPins[index]); // Set pin to push-pull

    // Enable clock for the GPIO pin
    if (pbPorts[index] == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (pbPorts[index] == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
}

void pb_0_isr(void) {
    
}

void pb_1_isr(void) {
    
}

uint8_t pb_get_state(uint8_t pushButton) {
    
    if (pushButton > (PUSH_BUTTONS - 1)) {
        return 255;
    }
    
    uint32_t pinIndex;

    switch (pushButton) {
        case 0:
            pinIndex = 0x01 << pbPins[0];
            return ((pbPorts[0]->IDR & pinIndex) == pinIndex ? GPIO_PIN_SET : GPIO_PIN_RESET);
        case 1:
            pinIndex = 0x01 << pbPins[1];
            return ((pbPorts[1]->IDR & pinIndex) == pinIndex ? GPIO_PIN_SET : GPIO_PIN_RESET);
        default:
            return 255;
    }
}