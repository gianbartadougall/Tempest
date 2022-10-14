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
// #include "tempest.h"
#include "utilities.h"
// #include "timer_ms.h"
#include "task_scheduler_1.h"
#include "piezo_buzzer.h"

/* STM32 Includes */
#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"

/**
 * @brief Interrupt handler for timer 1 and timer 15
 */
void TIM1_BRK_TIM15_IRQHandler(void) {
    // char m[50];
    // sprintf(m, "TIM 1 15 ---> CNT: %lu\tSR: %lu\r\n", TIM15->CNT, TIM15->SR);
    // debug_prints(m);
    // debug_prints("ISR TIM 1 15\r\n");

    // Check and clear overflow flag for TIM15
    if ((TIM15->SR & TIM_SR_UIF) == TIM_SR_UIF) {

        // Clear the UIF flag
        TIM15->SR = ~TIM_SR_UIF;

        /* Call required functions */
    }

    // Check and clear CCR1 flag for TIM15
    if ((TIM15->SR & TIM_SR_CC1IF) == TIM_SR_CC1IF) {

        // Clear the UIF flag
        TIM15->SR = ~TIM_SR_CC1IF;

        /* Call required functions */
        ts_isr();
    }

    // Check and clear CCR2 flag for TIM15
    if ((TIM15->SR & TIM_SR_CC2IF) == TIM_SR_CC2IF) {

        // Clear the UIF flag
        TIM15->SR = ~TIM_SR_CC2IF;

        /* Call required functions */
    }
}

/**
 * @brief Interrupt handler for timer 1 and timer 16
 *
 */
void TIM1_UP_TIM16_IRQHandler(void) {
    // char m[40];
    // sprintf(m, "TIMER 1 16 ISR: %lu\r\n", TIM1->SR);
    // debug_prints(m);

    // Check and clear overflow flag.
    if ((TIM16->SR & TIM_SR_UIF) == TIM_SR_UIF) {

        // Clear the UIF flag
        TIM16->SR = ~TIM_SR_UIF;

        /* Call required functions */
        piezo_buzzer_isr();
    }

    if ((TIM16->SR & TIM_SR_CC1IF) == TIM_SR_CC1IF) {

        // Clear the UIF flag
        TIM16->SR = ~TIM_SR_CC1IF;

        /* Call required functions */

        // Not sure if its a bug or not but the CCIF flag is set at some point
        // in the piezo buzzer ISR and it needs to be cleared here other wise
        // the system will enter an infinte loop
    }

    if ((TIM16->SR & TIM_SR_CC2IF) == TIM_SR_CC2IF) {

        // Clear the UIF flag
        TIM16->SR = ~TIM_SR_CC2IF;

        /* Call required functions */

        // Not sure if its a bug or not but the CCIF flag is set at some point
        // in the piezo buzzer ISR and it needs to be cleared here other wise
        // the system will enter an infinte loop
    }

    if ((TIM1->SR & TIM_SR_TIF) == TIM_SR_TIF) {

        // Clear the UIF flag
        TIM1->SR = ~TIM_SR_TIF;

        /* Call required functions */

        // Current bug in the system that sets TIF flag so to ensure an infinte ISR loop
        // doesn't occur, the TIF flag is cleared without doing anything else. To reproduce
        // this situation, remove the code that clears the TIF flag and then;
        // 1) Reset the system so it starts in automatic mode
        // 2) Enter manual override mode
        // 3) Hold down up button until motor starts moving
        // 4) Release up button. This ISR should now be calling in an infinite loop
    }

    if ((TIM1->SR & TIM_SR_UIF) == TIM_SR_UIF) {

        // Clear the UIF flag
        TIM1->SR = ~TIM_SR_UIF;

        /* Call required functions */

        // Current bug in the system that sets BIF flag so to ensure an infinte ISR loop
        // doesn't occur, the BIF flag is cleared without doing anything else. To reproduce
        // this situation, remove the code that clears the BIF flag and then;
        // 1) Reset the system so it starts in automatic mode
        // 2) Enter manual override mode
        // 3) Hold down up button until motor starts moving
        // 4) Release up button. This ISR should now be calling in an infinite loop
    }

    // char m[30];
    // sprintf(m, "TIM1: %lu\tTIM16: %lu\r\n", TIM1->SR, TIM16->SR);
    // debug_prints(m);
}

/**
 * @brief Interrupt handler for timer 1
 *
 */
void TIM1_TRG_COM_IRQHandler(void) {
    debug_prints("ISR TIM COM\r\n");
}

/**
 * @brief Capture compare interrupt handler for timer 1
 */
void TIM1_CC_IRQHandler(void) {
    debug_prints("ISR TIM CC\r\n");
    // Check for overflow flag

    if ((TIM1->SR & TIM_SR_UIF) == TIM_SR_UIF) {
        // Clear update event flag
        TIM1->SR = ~TIM_SR_UIF;
    }

    if ((TIM1->SR & TIM_SR_CC2IF) == TIM_SR_CC2IF) {

        // Clear capture compare flag
        TIM1->SR = ~TIM_SR_CC2IF;

        /* Call required functions */

        // Call encoder isr to turn motor off
        debug_prints("REACHED MINIMUM VALUE\r\n");
        // tempest_isr_encoder_at_min_value();
    }

    // Check if interrupt for CC3 was triggered
    if ((TIM1->SR & TIM_SR_CC3IF) == TIM_SR_CC3IF) {

        // Clear capture compare flag
        TIM1->SR = ~TIM_SR_CC3IF;

        /* Call required functions */

        // Call encoder isr to turn motor off
        debug_prints("REACHED MAXIMUM VALUE\r\n");
        // tempest_isr_encoder_at_max_value();
    }
}

/**
 * @brief Interrupt handler for timer 1
 *
 */
void TIM1_IRQHandler(void) {
    debug_prints("ISR TIM 1\r\n");
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

        // Return from function to ensure only the capture compare that is both enabled
        // and triggered is run
        return;
    }

    // // Check if interrupt for CC1 was triggered
    // if ((TIM2->SR & TIM_SR_CC1IF) == TIM_SR_CC1IF) {

    //     // Clear capture compare flag
    //     TIM2->SR = ~TIM_SR_CC1IF;

    //     /* Call required functions */

    //     if (pb0_triggered_early()) {
    //         encoder_isr_reset_min_value(); // Reset the minimum position of the blind
    //     } else {
    //         tempest_isr_force_blind_up(); // Force blind to move up
    //     }

    //     // Return from function to ensure only the capture compare that is both enabled
    //     // and triggered is run
    //     return;
    // }

    // if ((TIM2->SR & TIM_SR_CC2IF) == TIM_SR_CC2IF) {

    //     // Clear capture compare flag
    //     TIM2->SR = ~TIM_SR_CC2IF;

    //     /* Call required functions */
    //     if (pb1_triggered_early()) {
    //         encoder_isr_reset_max_value(); // Reset the maximum position of the blind
    //     } else {
    //         tempest_isr_force_blind_down(); // Force blind to move down
    //     }

    //     // Return from function to ensure only the capture compare that is both enabled
    //     // and triggered is run
    //     return;
    // }
}

/**
 * @brief Interrupt handler for timer 6
 *
 */
void TIM6_DAC_IRQHandler(void) {}

/**
 * @brief Interrupt handler for timer 7
 *
 */
void TIM7_IRQHandler(void) {}