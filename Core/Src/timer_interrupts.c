/**
 * @file timer_interrupts.h
 * @author Gian Barta-Dougall
 * @brief File to store interrupt handlers for timers for STM32L432KC mcu
 * @version 0.1
 * @date 2022-06-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */
/* Public Includes */

/* Private Includes */
#include "tempest.h"

/* STM32 Includes */
#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"

/**
 * @brief Interrupt handler for timer 1 and timer 15
 */
void TIM1_BRK_TIM15_IRQHandler(void) {

}

/**
 * @brief Interrupt handler for timer 1 and timer 16
 * 
 */
void TIM1_UP_TIM16_IRQHandler(void) {

    // Check and clear overflow flag.
    if ((TIM16->SR & TIM_SR_UIF) == TIM_SR_UIF) {

        // Clear the UIF flag
        TIM16->SR &= ~TIM_SR_UIF;

        /* Call required functions */
        piezo_buzzer_isr();
    }
}

/**
 * @brief Interrupt handler for timer 1
 * 
 */
void TIM1_TRG_COM_IRQHandler(void) {

}

/**
 * @brief Capture compare interrupt handler for timer 1
 */
void TIM1_CC_IRQHandler(void) {

    // Check for overflow flag

    if ((TIM1->SR & TIM_SR_CC2IF) == TIM_SR_CC2IF) {
    
        // Clear capture compare flag
        TIM1->SR &= ~TIM_SR_CC2IF;

        /* Call required functions */
    
        // Call encoder isr to turn motor off
        tempest_isr_encoder_at_min_value();
    }

    // Check if interrupt for CC3 was triggered
    if ((TIM1->SR & TIM_SR_CC3IF) == TIM_SR_CC3IF) {
    
        // Clear capture compare flag
        TIM1->SR &= ~TIM_SR_CC3IF;

        /* Call required functions */
        
        // Call encoder isr to turn motor off
        tempest_isr_encoder_at_max_value();
    }

}

/**
 * @brief Interrupt handler for timer 1
 * 
 */
void TIM1_IRQHandler(void) {

}

/**
 * @brief Interrupt handler for timer 2
 * 
 */
void TIM2_IRQHandler(void) {

    // Check for event
    if ((TIM2->SR & TIM_SR_UIF) == TIM_SR_UIF) {

        // Clear UIF flag
        TIM2->SR = ~TIM_SR_UIF;

    }

    // Check if interrupt for CC1 was triggered
    if ((TIM2->SR & TIM_SR_CC1IF) == TIM_SR_CC1IF) {
    
        // Clear capture compare flag
        TIM2->SR = ~TIM_SR_CC1IF;

        /* Call required functions */

        if (pb0_triggered_early()) {
            // Reset min point on blind
            debug_prints("Resetting minimum point\r\n");
        } else {

            // Move motor as button has been held down long enough
            debug_prints("Moving motor up\r\n");
        }
    }

    if ((TIM2->SR & TIM_SR_CC2IF) == TIM_SR_CC2IF) {
    
        // Clear capture compare flag
        TIM2->SR = ~TIM_SR_CC2IF;
        
        /* Call required functions */

        if (pb1_triggered_early()) {
            // Reset max point on blind
            debug_prints("Resetting maximum point\r\n");
        } else {

            // Move motor as button has been held down long enough
            debug_prints("Moving motor down\r\n");
        }
    }

}

/**
 * @brief Interrupt handler for timer 6
 * 
 */
void TIM6_DAC_IRQHandler(void) {

}

/**
 * @brief Interrupt handler for timer 7
 * 
 */
void TIM7_IRQHandler(void) {

}