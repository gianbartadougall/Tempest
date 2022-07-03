/**
 * @file timer_ms.c
 * @author Gian Barta-Dougall
 * @brief Generic millisecond timer
 * @version 0.1
 * @date 2022-07-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/* Public Includes */

/* Private Includes */
#include "timer_ms.h"
#include "debug_log.h"
#include "interrupts_config.h"

/* STM32 Includes */

/* Private #defines */
#define MS_TIMER TIM16
#define MS_TIMER_FREQUENCY 1000
#define MS_TIMER_MAX_VALUE 65535 // Set timer to have maximum period 
#define MS_TIMER_CLK_ENABLE() __HAL_RCC_TIM16_CLK_ENABLE()

/* Variable Declarations */

/* Function prototypes */

void timer_ms_init(void) {

    // Verify the frequency of the system clock is within range to create a millisecond timer
    if (SystemCoreClock > 65535000) {
        debug_prints("TIMER MS error. System clock frequency too high to generate a millisecond timer on TIM3\r\n");
    }

    /* Configure timer */

     // Enable timer clock
    MS_TIMER_CLK_ENABLE();

    // Set counter to up counting
    MS_TIMER->CR1 &= ~TIM_CR1_DIR;

    MS_TIMER->PSC = (SystemCoreClock / MS_TIMER_FREQUENCY) - 1;
    MS_TIMER->ARR = MS_TIMER_MAX_VALUE;
    
    // Enable capture compare on CCR1 and CCR2 and interrupt on counter overflow
    MS_TIMER->DIER |= (TIM_DIER_CC1IE | TIM_DIER_CC2IE | TIM_DIER_UIE);

    // Set capture compare mode to output for CH1 and CH2
    MS_TIMER->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_CC2S); // Set capture compare to output
    MS_TIMER->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M); // Set output compare mode to Frozen

    // Set CCR1 and CCR2 registers
    MS_TIMER->CCR1 = 5000; // Set ISR sequence one to occur at 20 seconds
    MS_TIMER->CCR2 = 5001; // Set ISR sequence two to occur 1ms after first sequence

    // Enable interrupt handler
    HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, TIM16_ISR_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
}

void timer_ms_enable(void) {
    MS_TIMER->CR1 |= (TIM_CR1_CEN);
}

void timer_ms_disable(void) {
    MS_TIMER->CR1 &= ~(TIM_CR1_CEN);
}

void 