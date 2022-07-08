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
#define TIMER_MS TIM15 // Timer being used to run millisecond counter
#define MS_TIMER_FREQUENCY 1000 // The frequency of the counter
#define MS_TIMER_MAX_VALUE 65535 // Set timer to have maximum period 
#define MS_TIMER_CLK_ENABLE() __HAL_RCC_TIM15_CLK_ENABLE() // CLK enable for timer

// Defining free and start index's for IDs
#define MS_TIMER_ID_FREE 50
#define MS_TIMER_ID_START_INDEX (MS_TIMER_ID_FREE + 1)

#define CH1 TIMER_MS_CHANNEL_1
#define CH2 TIMER_MS_CHANNEL_2

// The number of channels used by the timer
#define NUM_CHANNELS 2

/* Private Enumeration Declarations */

enum TimeMsTaskStatus {TMS_TASK_STATUS_PAUSED, TMS_TASK_STATUS_QUEUED, TMS_TASK_STATUS_RUNNING};

/* Variable Declarations */

// List to hold information for each channel
TimerMsChannel Channels[NUM_CHANNELS];
uint8_t numIdsAssigned = 0;

#define CH(chnl) (Channels[chnl])
#define CH_QUEUE(chnl, i) (Channels[chnl].queue[i])
#define CH_SIZE(chnl) (Channels[chnl].size)

/* Function prototypes */
void (*ccr1_isr) (void) = NULL;
void (*ccr2_isr) (void) = NULL;
void timer_ms_add_task_to_queue(uint8_t chnl, TimerMsTask* task);
void timer_ms_start_next_delay(uint8_t chnl);
void timer_ms_remove_task(uint8_t chnl, uint8_t index);
void timer_ms_priority_cancel_current_task(uint8_t chnl);
void timer_ms_channel_reset(uint8_t chnl);
void timer_ms_reset_first_task(uint8_t chnl);
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
    TIMER_MS->CR1 &= ~TIM_CR1_DIR;

    TIMER_MS->PSC = (SystemCoreClock / MS_TIMER_FREQUENCY) - 1;
    TIMER_MS->ARR = MS_TIMER_MAX_VALUE;
    
    // Disable all interrupts
    TIMER_MS->DIER &= 0x00;

    // Set capture compare mode to output for CH1 and CH2
    TIMER_MS->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_CC2S); // Set capture compare to output
    TIMER_MS->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M); // Set output compare mode to Frozen

    // Enable interrupt handler
    HAL_NVIC_SetPriority(TIM1_BRK_TIM15_IRQn, TIM16_ISR_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);

    // Reset both channels
    timer_ms_channel_reset(CH1);
    timer_ms_channel_reset(CH2);
}

void timer_ms_enable(void) {
    TIMER_MS->EGR |= (TIM_EGR_UG); // Init counter to 0 and update all registers
    TIMER_MS->CR1 |= (TIM_CR1_CEN); // Enable counter
}

void timer_ms_disable(void) {
    TIMER_MS->CR1 &= ~(TIM_CR1_CEN); // Disable counter
}

void timer_ms_init_task(TimerMsTask* task, void (*isrs[TIMER_MS_MAX_TASK_SIZE])(void), 
        uint16_t delays[TIMER_MS_MAX_TASK_SIZE], uint8_t numberOfDelays, 
        uint8_t actionOnPriorityCancel, uint8_t priority,
        uint8_t actionOnCompletion) {
    
    // Generate ID for handle
    task->id = MS_TIMER_ID_START_INDEX + numIdsAssigned;
    numIdsAssigned++;

    task->size = numberOfDelays;
    task->index = 0;
    task->actionOnPriorityCancel = actionOnPriorityCancel;
    task->priority = priority;
    task->actionOnCompletion = actionOnCompletion;

    // Initialise the list of ISRs to handle
    for (uint8_t i = 0; i < numberOfDelays; i++) {
        task->isrs[i] = isrs[i];
    }

    // Initialise the list of delays to handle
    for (uint8_t i = 0; i < numberOfDelays; i++) {
        task->delays[i] = delays[i];
    }
}

void timer_ms_add_task(TimerMsTask* task) {

    // Exit if both channels are full
    if ((CH_SIZE(CH1) == TIMER_MS_MAX_QUEUE_SIZE) && (CH_SIZE(CH2) == TIMER_MS_MAX_QUEUE_SIZE)) {
        return;
    }

    /* Add task to a queue that is free if possible */
    if (CH_SIZE(CH1) == 0) {
        timer_ms_add_task_to_queue(CH1, task);
        timer_ms_start_next_delay(CH1);
        return;
    }

    if (CH_SIZE(CH2) == 0) {
        timer_ms_add_task_to_queue(CH2, task);
        timer_ms_start_next_delay(CH2);
        return;
    }

    /* Cancel the lowest pripority task of either channel if the
        new task to run has a higher priority */
    uint8_t cancelCh1 = (task->priority > CH_QUEUE(CH1, 0)->priority) ? 1 : 0;
    uint8_t cancelCh2 = (task->priority > CH_QUEUE(CH2, 0)->priority) ? 1 : 0;
    uint8_t channelToCancel = 0;

    // Cancel the lowest priority task if the new task has a higher priority then both
    // the tasks currently in Channel 1 and Channel 2
    if (cancelCh1 && cancelCh2) {
        if (CH_QUEUE(CH1, 0)->priority > CH_QUEUE(CH2, 0)->priority) {
            channelToCancel = 2;
        } else {
            channelToCancel = 1;
        }
    } else if (cancelCh1 == 1) {
        channelToCancel = 1;
    } else if (cancelCh2 == 1) {
        channelToCancel = 2;
    }

    // Cancel current task on Channel 1 if required 
    if (channelToCancel == 1) {
        // debug_prints("Cancelling and adding to CH1 task\r\n");
        timer_ms_priority_cancel_current_task(CH1);
        timer_ms_add_task_to_queue(CH1, task);
        timer_ms_start_next_delay(CH1);
        return;
    }

    // Cancel current task on Channel 2 if required
    if (channelToCancel == 2) {
        // debug_prints("Cancelling and adding to CH2 task\r\n");
        timer_ms_priority_cancel_current_task(CH2);
        timer_ms_add_task_to_queue(CH2, task);
        timer_ms_start_next_delay(CH2);
        return;
    }

    // Check if any queue currently has a task on repeat
    if ((CH_QUEUE(CH1, 0)->actionOnCompletion != TIMER_MS_TASK_REPEAT) && 
        (CH_QUEUE(CH2, 0)->actionOnCompletion == TIMER_MS_TASK_REPEAT)) {
            
            timer_ms_add_task_to_queue(CH1, task);
            return;
    }

    if ((CH_QUEUE(CH1, 0)->actionOnCompletion == TIMER_MS_TASK_REPEAT) && 
        (CH_QUEUE(CH2, 0)->actionOnCompletion != TIMER_MS_TASK_REPEAT)) {
            
            timer_ms_add_task_to_queue(CH2, task);
            return;
    } 

    /* The new task has a lower priority than the current tasks running. Add the task
        to the channel with the lowest number of tasks to run */
    if (CH_SIZE(CH1) < CH_SIZE(CH2)) {
        timer_ms_add_task_to_queue(CH1, task);
    } else {
        timer_ms_add_task_to_queue(CH2, task);
    }
}

void timer_ms_isr(uint8_t chnl) {

    if (CH_SIZE(chnl) != 0) {

        // Run the next ISR
        (*CH_QUEUE(chnl, 0)->isrs[CH_QUEUE(chnl, 0)->index]) ();
        CH_QUEUE(chnl, 0)->index++;

        // Start the next delay in list if one exists
        if (CH_QUEUE(chnl, 0)->index < CH_QUEUE(chnl, 0)->size) {
            timer_ms_start_next_delay(chnl);
            return;
        }

        // No more delays are left in the current task. Confirm
        // whether task is to be removed or repeated
        if (CH_QUEUE(chnl, 0)->actionOnCompletion == TIMER_MS_TASK_REPEAT) {

            // Reset index and start next delay
            CH_QUEUE(chnl, 0)->index = 0;
            timer_ms_start_next_delay(chnl);
            return;
        }
        
        // Task is finished. Removed task from queue
        timer_ms_remove_task(chnl, 0);

        // Check if there is another task to run
        if (CH_SIZE(chnl) > 0) {
            timer_ms_start_next_delay(chnl);
            return;
        }
    }

    // Reset Channel 1
    timer_ms_channel_reset(chnl);
}

uint8_t timer_ms_task_is_in_queue(TimerMsTask* task) {

    // Check tasks in all channels
    for (uint chnl = CH1; chnl < NUM_CHANNELS; chnl++) {
        for (uint8_t i = 0; i < CH_SIZE(chnl); i++) {
            if (CH_QUEUE(chnl, i)->id == task->id) {
                return 1;
            }
        }
    }

    return 0;
}

void timer_ms_cancel_task(TimerMsTask* task) {

    // Check tasks in all channels
    for (uint chnl = CH1; chnl < NUM_CHANNELS; chnl++) {
        for (uint8_t i = 0; i < CH_SIZE(chnl); i++) {
            if (CH_QUEUE(chnl, i)->id == task->id) {
                
                // Remove task if ID matches
                timer_ms_remove_task(chnl, i);
                break;
            }
        }
    }
}

/* Private Functions */

void timer_ms_add_task_to_queue(uint8_t chnl, TimerMsTask* task) {

    // Create a new queue to store tasks temporarily whilst sorting
    TimerMsTask* sortedQueue[TIMER_MS_MAX_QUEUE_SIZE];
    uint8_t newTaskAdded = 0;
    int i;

    // Loop through queue and determine position to add new task based on priority
    for (i = 0; i < CH_SIZE(chnl); i++) {

        // Add new task to index i if it has a higher priority than the current
        // task at that index
        if (task->priority > CH_QUEUE(chnl, i)->priority) {
            sortedQueue[i] = task;
            newTaskAdded = 1;
            break;
        }

        sortedQueue[i] = CH_QUEUE(chnl, i);
    }

    // Add new task to list if it wasn't add in previous for loop
    if (newTaskAdded == 0) {
        sortedQueue[CH_SIZE(chnl)] = task;
    } else {

        // New task has been added. Add remaining tasks to queue that were skipped
        // in previous for loop
        uint8_t start = 1 + i; // Add 1 to account for new task that is in the list
        uint8_t end = CH_SIZE(chnl) + 1; // Add 1 to account for new task that is in the list
        
        for (uint8_t j = start; j < end; j++) {

            // Subtract one from that actual channel queue as it was skipped in previous for
            // loop
            sortedQueue[j] = CH_QUEUE(chnl, j - 1);
        }
    }

    // Increment channel size
    CH_SIZE(chnl)++;

    // Copy sorted list back into Channel 1 queue
    for (int k = 0; k < CH_SIZE(chnl); k++) {
        CH_QUEUE(chnl, k) = sortedQueue[k];
    }

    // Update task status
    task->status = TMS_TASK_STATUS_QUEUED;
}

void timer_ms_start_next_delay(uint8_t chnl) {

    uint16_t delayTime;
    
    // If the task is currently paused, use the delay remaining
    if (CH_QUEUE(chnl, 0)->status == TMS_TASK_STATUS_PAUSED) {
        delayTime = CH_QUEUE(chnl, 0)->delayRemaining;
    } else {
        delayTime = CH_QUEUE(chnl, 0)->delays[CH_QUEUE(chnl, 0)->index];
    }

    // Set the task status to running
    CH_QUEUE(chnl, 0)->status = TMS_TASK_STATUS_RUNNING;
    
    // Initialise interrupts
    if (chnl == CH1) {
        TIMER_MS->CCR1 = timer_ms_calculate_end_time(delayTime);
        TIMER_MS->SR = ~(TIM_SR_CC1IF); // Clear any outstanding interrupts
        TIMER_MS->DIER |= (TIM_DIER_CC1IE); // Enable interrupts on channel 1
    }

    if (chnl == CH2) {
        TIMER_MS->CCR2 = timer_ms_calculate_end_time(delayTime);
        TIMER_MS->SR = ~(TIM_SR_CC2IF); // Clear any outstanding interrupts
        TIMER_MS->DIER |= (TIM_DIER_CC2IE); // Enable interrupts on channel 2
    }
}

void timer_ms_remove_task(uint8_t chnl, uint8_t index) {

    // Reset task back to its default state. This ensures if it is added
    // to the queue again, it has the correct values
    CH_QUEUE(chnl, index)->index = 0;

    // Start one index above the index of the task to remove
    for (int i = 1 + index; i < CH_SIZE(chnl); i++) {

        // Move each task up 1 index
        CH_QUEUE(chnl, i - 1) = CH_QUEUE(chnl, i);
    }

    // Decrement the queue size
    CH_SIZE(chnl)--;
}

void timer_ms_priority_cancel_current_task(uint8_t chnl) {

    // Confirm the channel has a running
    if (CH_SIZE(chnl) == 0) {
        return;
    }

    switch (CH_QUEUE(chnl, 0)->actionOnPriorityCancel) {
        case TIMER_MS_TASK_CANCEL:
            timer_ms_remove_task(chnl, 0);
            break;
        
        case TIMER_MS_TASK_RESET:
            timer_ms_reset_first_task(chnl);
            break;
        
        case TIMER_MS_TASK_PAUSE:

            if (TIMER_MS->CCR1 < TIMER_MS->CNT) {
                CH_QUEUE(chnl, 0)->delayRemaining = (TIMER_MS->ARR - TIMER_MS->CNT) + TIMER_MS->CCR1; 
            } else {
                CH_QUEUE(chnl, 0)->delayRemaining = TIMER_MS->CCR1 - TIMER_MS->CNT; 
            }

            // Set paused flag on this task
            CH_QUEUE(chnl, 0)->status = TMS_TASK_STATUS_PAUSED;
            break;
    default:
        break;
    }

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
    uint16_t currentCount = TIMER_MS->CNT;

    // Calculate counts left before the timer resets
    uint16_t difference = TIMER_MS->ARR - currentCount;

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
void timer_ms_channel_reset(uint8_t chnl) {

    // Reset channel size
    CH_SIZE(chnl) = 0;
    
    // Disbale interrupts for given channel
    if (chnl == CH1) {
        TIMER_MS->DIER &= ~(TIM_DIER_CC1IE);
    }

    if (chnl == CH2) {
        TIMER_MS->DIER &= ~(TIM_DIER_CC2IE);
    }
}

void timer_ms_reset_first_task(uint8_t chnl) {
    CH_QUEUE(chnl, 0)->index = 0;
}