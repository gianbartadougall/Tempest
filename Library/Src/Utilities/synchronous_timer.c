/**
 * @file synchronous_timer.c
 * @author Gian Barta-Dougall
 * @brief System file for synchronous_timer
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "synchronous_timer.h"
#include "hardware_config.h"
#include "log.h"

/* Private STM Includes */
#include "stm32l4xx.h"

/* Private #defines */
#define SYNC_TIMER HC_SYNC_TIMER

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */

/* Public Functions */

void synchronous_timer_enable(void) {
    SYNC_TIMER->EGR |= (TIM_EGR_UG);  // Reset counter to 0 and update all registers
    SYNC_TIMER->DIER |= TIM_DIER_UIE; // Enable interrupts
    SYNC_TIMER->CR1 |= TIM_CR1_CEN;   // Start the timer
}

void synchronous_timer_disable(void) {
    SYNC_TIMER->DIER &= 0x00;          // Disable all interrupts
    SYNC_TIMER->CR1 &= ~(TIM_CR1_CEN); // Disbable timer
}

/* Private Functions */
