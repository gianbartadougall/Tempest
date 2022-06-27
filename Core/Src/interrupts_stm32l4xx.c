
/* STM32 Includes */
#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"

/* Private Includes */
#include "interrupts_stm32l4xx.h"
#include "tempest.h"

/**
 * @brief Interrupt routine for EXTI1 
 * 
 */
void EXTI0_IRQHandler(void) {
    // Clear the pending interrupt call
    NVIC_ClearPendingIRQ(EXTI1_IRQn);
    
    // Function calls for pin 4
    if ((EXTI->PR1 & EXTI_PR1_PIF0) == EXTI_PR1_PIF0) {

        // Clear the interrupt flag
        EXTI->PR1 |= EXTI_PR1_PIF0;

        /* Call required functions */
        pb_0_isr();
        // *************************
    }

}

/**
 * @brief Interrupt routine for EXTI1 
 * 
 */
void EXTI1_IRQHandler(void) {
    // Clear the pending interrupt call
    NVIC_ClearPendingIRQ(EXTI1_IRQn);
    
    // Function calls for pin 4
    if ((EXTI->PR1 & EXTI_PR1_PIF1) == EXTI_PR1_PIF1) {

        // Clear the interrupt flag
        EXTI->PR1 |= EXTI_PR1_PIF1;

        /* Call required functions */
        pb_1_isr();
        // *************************
    }
    
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
}

/**
 * @brief Interrupt routine for EXTI2
 * 
 */
void EXTI2_IRQHandler(void) {
    
}

/**
 * @brief Interrupt routine for EXTI3
 * 
 */
void EXTI3_IRQHandler(void) {
    
}

/**
 * @brief Interrupt routine for EXTI4
 * 
 */
void EXTI4_IRQHandler(void) {

    // Clear the pending interrupt call
    NVIC_ClearPendingIRQ(EXTI4_IRQn);
    
    // Function calls for pin 4
    if ((EXTI->PR1 & EXTI_PR1_PIF4) == EXTI_PR1_PIF4) {

        // Clear the interrupt flag
        EXTI->PR1 |= EXTI_PR1_PIF4;

        /* Call required functions */
        encoder_isr();
        // *************************
    }
}

/**
 * @brief Interrupt routine for EXTI5 - EXTI9
 * 
 */
void EXTI9_5_IRQHandler(void) {
    // Clear pending interrupt call
    NVIC_ClearPendingIRQ(EXTI9_5_IRQn);

    // Check if interrupt was for manual override pin
    if ((EXTI->PR1 & EXTI_PR1_PIF5) == (EXTI_PR1_PIF5)) {

        // Clear the pending interrupt
        EXTI->PR1 |= EXTI_PR1_PIF5;

        // Check if the interrupt was a rising or falling edge
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == 1) {
            set_manual_override();
        } else {
            clear_manual_override();
        }
    }

}

/**
 * @brief Interrupt routine for EXTI10 - EXTI15
 * 
 */
void EXTI15_10_IRQHandler(void) {
    
    // Clear the pending interrupt call
    NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
    
    // Function calls for pin 11
    if ((EXTI->PR1 & EXTI_PR1_PIF11) == EXTI_PR1_PIF11) {

        // Clear the interrupt flag
        EXTI->PR1 |= EXTI_PR1_PIF11;

        /* Call required functions */
        // encoder_isr();
        // *************************
    }
}

void TIM2_IRQHandler(void) {

}

void TIM1_UP_TIM16_IRQHandler(void) {

    // Check and clear overflow flag.
    if((TIM16->SR & TIM_SR_UIF) == TIM_SR_UIF) {

        // Clear the UIF flag
        TIM16->SR &= ~TIM_SR_UIF;

        /* Call required functions */
        piezo_buzzer_isr();
    }
}