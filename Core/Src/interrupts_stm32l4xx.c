
/* STM32 Includes */
#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"

/* Private Includes */
#include "interrupts_stm32l4xx.h"
#include "debug_log.h"
#include "encoder.h"
#include "pushbutton.h"

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