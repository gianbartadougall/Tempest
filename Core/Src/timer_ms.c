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
#define MS_TIMER TIM15 // Timer being used to run millisecond counter
#define MS_TIMER_FREQUENCY 1000 // The frequency of the counter
#define MS_TIMER_MAX_VALUE 65535 // Set timer to have maximum period 
#define MS_TIMER_CLK_ENABLE() __HAL_RCC_TIM15_CLK_ENABLE() // CLK enable for timer

// Defining free and start index's for IDs
#define MS_TIMER_ID_FREE 50
#define MS_TIMER_ID_START_INDEX (MS_TIMER_ID_FREE + 1)

/* Variable Declarations */

// Handles for both channels
TimerMsHandle Channel1;
TimerMsHandle Channel2;
uint8_t numIdsAssigned = 0;

/* Function prototypes */
void (*ccr1_isr) (void) = NULL;
void (*ccr2_isr) (void) = NULL;
void reset_channel_1(void);
void reset_channel_2(void);
void timer_ms_ch1_set_next_delay(void);
void timer_ms_ch2_set_next_delay(void);
void timer_ms_print_ch1_state(void);
void timer_ms_print_ch2_state(void);
uint16_t timer_ms_calculate_end_time(uint16_t delay);

/* Public Functions */

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
    
    // Disable all interrupts
    MS_TIMER->DIER &= 0x00;

    // Set capture compare mode to output for CH1 and CH2
    MS_TIMER->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_CC2S); // Set capture compare to output
    MS_TIMER->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M); // Set output compare mode to Frozen

    // Enable interrupt handler
    HAL_NVIC_SetPriority(TIM1_BRK_TIM15_IRQn, TIM16_ISR_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);

    // Reset both channels
    reset_channel_1();
    reset_channel_2();
}

void timer_ms_enable(void) {
    MS_TIMER->EGR |= (TIM_EGR_UG); // Init counter to 0 and update all registers
    MS_TIMER->CR1 |= (TIM_CR1_CEN); // Enable counter
}

void timer_ms_disable(void) {
    MS_TIMER->CR1 &= ~(TIM_CR1_CEN); // Disable counter
}

void timer_ms_init_handle(TimerMsHandle* handle, void (*isrs[TIMER_MS_MAX_DELAYS_PER_CHANNEL])(void), 
        uint16_t delays[TIMER_MS_MAX_DELAYS_PER_CHANNEL], uint8_t numberOfDelays) {
    
    // Generate ID for handle
    handle->handleID = MS_TIMER_ID_START_INDEX + numIdsAssigned;
    numIdsAssigned++;

    // Initialise the list of ISRs to handle
    for (uint8_t i = 0; i < numberOfDelays; i++) {
        handle->isrs[i] = isrs[i];
    }

    // Initialise the list of delays to handle
    for (uint8_t i = 0; i < numberOfDelays; i++) {
        handle->delays[i] = delays[i];
    }

    // Initialise the number of delays
    handle->numberOfDelays = numberOfDelays;

    // Set the index to 0
    handle->index = 0;
}

uint8_t timer_ms_set_delay(TimerMsHandle* handle) {

    if (Channel1.handleID == MS_TIMER_ID_FREE) {

        /* Copy all parameters from handle to Channel 1 */

        Channel1.handleID = handle->handleID;
        Channel1.numberOfDelays = handle->numberOfDelays;
        Channel1.index = handle->index;

        // Copy the list of ISRs to channel 1 struct
        for (uint8_t i = 0; i < handle->numberOfDelays; i++) {
            Channel1.isrs[i] = handle->isrs[i];
        }

        // Copy list of delays to channel 1 struct
        for (uint8_t i = 0; i < handle->numberOfDelays; i++) {
            Channel1.delays[i] = handle->delays[i];
        }

        // Initialise the first dlay
        timer_ms_ch1_set_next_delay();

        // Initialise interrupts
        MS_TIMER->SR = ~(TIM_SR_CC1IF); // Clear any outstanding interrupts
        MS_TIMER->DIER |= (TIM_DIER_CC1IE); // Enable interrupts on channel 1

    } else if (Channel2.handleID == MS_TIMER_ID_FREE) {

        /* Copy all parameters from handle to Channel 1 */

        Channel2.handleID = handle->handleID;
        Channel2.numberOfDelays = handle->numberOfDelays;
        Channel2.index = handle->index;

        // Copy the list of ISRs to channel 1 struct
        for (uint8_t i = 0; i < handle->numberOfDelays; i++) {
            Channel2.isrs[i] = handle->isrs[i];
        }

        // Copy list of delays to channel 1 struct
        for (uint8_t i = 0; i < handle->numberOfDelays; i++) {
            Channel2.delays[i] = handle->delays[i];
        }

        // Initialise the first dlay
        timer_ms_ch2_set_next_delay();

        // Initialise interrupts
        MS_TIMER->SR = ~(TIM_SR_CC2IF); // Clear any outstanding interrupts
        MS_TIMER->DIER |= (TIM_DIER_CC2IE); // Enable interrupts on channel 1

    } else {
        return TASK_REJECTED;
    }

    return TASK_ACCEPTED;
}

void timer_ms_ch1_set_next_delay(void) {
    // Calculate the required CCR1 value
    MS_TIMER->CCR1 = timer_ms_calculate_end_time(Channel1.delays[Channel1.index]);
}

void timer_ms_ch2_set_next_delay(void) {
    // Calculate the required CCR2 value
    MS_TIMER->CCR2 = timer_ms_calculate_end_time(Channel2.delays[Channel2.index]);
}

void timer_ms_cancel_delay(TimerMsHandle* handle) {

    // Cancel channel 1 delay if handle matches current ID
    if (Channel1.handleID == handle->handleID) {
        reset_channel_1();
    }

    if (Channel2.handleID == handle->handleID) {
        reset_channel_2();
    }
}

void timer_ms_isr_ch1(void) {

    // Check if Channel is currently running a delay
    if (Channel1.handleID != MS_TIMER_ID_FREE) {

        (*Channel1.isrs[Channel1.index])(); // Run next ISR in list
        Channel1.index++; // Increment index

        // Check if there is another delay in list to run 
        if (Channel1.index < Channel1.numberOfDelays) {
            
            // Set the next delay in list
            timer_ms_ch1_set_next_delay();
            
            return; // Return from ISR before resetting channel
        }
    }

    // No more ISRs in list to run. Reset channel 1.
    reset_channel_1();

}

void timer_ms_isr_ch2(void) {

    // Check if Channel is currently running a delay
    if (Channel2.handleID != MS_TIMER_ID_FREE) {

        (*Channel2.isrs[Channel2.index])(); // Run next ISR in list
        Channel2.index++; // Increment index

        // Check if there is another delay in list to run 
        if (Channel2.index < Channel2.numberOfDelays) {

            // Set the next delay in list
            timer_ms_ch2_set_next_delay();
            
            return; // Return from ISR before resetting channel
        }
    }

    // No more ISRs in list to run. Reset channel 2.
    reset_channel_2();
}

uint8_t timer_ms_ch1_is_free(void) {
    return (Channel1.handleID == MS_TIMER_ID_FREE) ? 1 : 0;
}

uint8_t timer_ms_ch2_is_free(void) {
    return (Channel2.handleID == MS_TIMER_ID_FREE) ? 1 : 0;
}

uint8_t timer_ms_handle_in_use(TimerMsHandle* handle) {

    // Returns 1 if a channel is currently performing a delay for the handle 
    if ((Channel1.handleID == handle->handleID) || (Channel2.handleID == handle->handleID)) {
        return 1;
    }

    return 0;
}

void timer_ms_print_handle_status(TimerMsHandle* handle) {

    if (handle->handleID == Channel1.handleID) {
        timer_ms_print_ch1_state();
    } else if (handle->handleID == Channel1.handleID) {
        timer_ms_print_ch2_state();
    } else {
        char m[50];
        sprintf(m, "Timer CNT: %lu. Handle not in use\r\n", MS_TIMER->CNT);
        debug_prints(m);
    }
}

/* Private Functions */

/**
 * @brief Prints the current state of channel 1
 */
void timer_ms_print_ch1_state(void) {
    
    uint16_t timeToNextISR;
    uint16_t currentCount = MS_TIMER->CNT;
    uint16_t currentCCR1 = MS_TIMER->CCR1; 

    if (currentCount <= currentCCR1) {
        timeToNextISR = currentCCR1 - currentCount;
    } else {
        timeToNextISR = (MS_TIMER_MAX_VALUE - currentCount) + currentCCR1;
    }

    char m[150];
    sprintf(m, "Channel 1:\r\n \
                ID = %i \r\n \
                Index: %i \r\n \
                Length: %i \r\n \
                Next ISR in %i ms\r\n", Channel1.handleID, Channel1.index, Channel1.numberOfDelays, timeToNextISR);
    debug_prints(m);
}

/**
 * @brief Prints the current state of channel 2
 */
void timer_ms_print_ch2_state(void) {
    
    uint16_t timeToNextISR;
    uint16_t currentCount = MS_TIMER->CNT;
    uint16_t currentCCR2 = MS_TIMER->CCR2; 

    if (currentCount <= currentCCR2) {
        timeToNextISR = currentCCR2 - currentCount;
    } else {
        timeToNextISR = (MS_TIMER_MAX_VALUE - currentCount) + currentCCR2;
    }

    char m[150];
    sprintf(m, "Channel 2:\r\n \
                ID = %i \r\n \
                Index: %i \r\n \
                Length: %i \r\n \
                Next ISR in %i ms\r\n", Channel2.handleID, Channel2.index, Channel2.numberOfDelays, timeToNextISR);
    debug_prints(m);
}

/**
 * @brief Calcualtes the value the CCR register should have for the correct delay
 * time to be implemented
 * 
 * @param delay The delay length
 * @return uint16_t The value the CCR register should take for the delay to occur
 */
uint16_t timer_ms_calculate_end_time(uint16_t delay) {

    // Store the current count
    uint16_t currentCount = MS_TIMER->CNT;

    // Calculate counts left before the timer resets
    uint16_t difference = MS_TIMER->ARR - currentCount;

    // Determine correct CCR value based on whether the delay will occur
    // before or after the counter resets
    if (difference < delay) {
        return delay - difference;
    }

    return currentCount + delay;
}

/**
 * @brief Resets channel 1 so it can be used for a new delay
 */
void reset_channel_1(void) {
    MS_TIMER->DIER &= ~(TIM_DIER_CC1IE); // Disable interrupts
    Channel1.handleID = MS_TIMER_ID_FREE;
}

/**
 * @brief Resets channel 2 so it can be used for a new delay
 */
void reset_channel_2(void) {
    MS_TIMER->DIER &= ~(TIM_DIER_CC2IE); // Disable interrupts
    Channel2.handleID = MS_TIMER_ID_FREE;
}