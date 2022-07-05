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
#ifndef TIMER_MS_H
#define TIMER_MS_H

/* STM32 Includes */
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"

/* Public Structs */

#define TIMER_MS_MAX_DELAYS_PER_CHANNEL 3
/**
 * @brief Handle for delay tasks. This handle allows a running delay to be cancelled
 * if required
 */
typedef struct TimerMsHandle {
    uint8_t handleID;
    void (*isrs[TIMER_MS_MAX_DELAYS_PER_CHANNEL]) (void); // Set the maximum number of ISRs to 3
    uint16_t delays[TIMER_MS_MAX_DELAYS_PER_CHANNEL];
    uint8_t numberOfDelays;
    uint8_t index;
} TimerMsHandle;

/* Public Enumerations */

// Status symbols to allow user to know whether a delay has been accepted or not
enum TimerMsStatus {TASK_ACCEPTED, TASK_REJECTED};

/**
 * @brief Initialise the timer hardware. This function does not start the timer
 */
void timer_ms_init(void);

/**
 * @brief Initialise parameters into Timer Handle struct. This function also assigns
 * an ID to the handle
 * 
 * @param handle The pointer to the handle to be initialised
 * @param isrs The list of ISRs that need to be run (in order)
 * @param delays The list of delays to wait between each ISR (in order)
 * @param numberOfDelays The size of the list of delays. Note this should be the
 * same size as the list of ISRs 
 */
void timer_ms_init_handle(TimerMsHandle* handle, void (*isrs[TIMER_MS_MAX_DELAYS_PER_CHANNEL])(void), 
        uint16_t delays[TIMER_MS_MAX_DELAYS_PER_CHANNEL], uint8_t numberOfDelays);

/**
 * @brief Initialise the Channels to run the ISRs at the delay points specified by the handle
 * 
 * @param handle The handle containing the information
 * @return uint8_t 1 if there is a free channel and the delay can be run else 0
 */
uint8_t timer_ms_set_delay(TimerMsHandle* handle);

/**
 * @brief Starts the timer
 */
void timer_ms_enable(void);

/**
 * @brief Stops the counter from updating. Does not cancel any delays that have
 * currently been assigned
 * 
 */
void timer_ms_disable(void);

/**
 * @brief Cancels a delay that is currently in operation. This function has no affect if the
 * handle passed does not point to a channel
 * 
 * @param handle Handle to the delay that needs to be cancelled
 */
void timer_ms_cancel_delay(TimerMsHandle* handle);

/**
 * @brief ISR for channel 1. Calls the ISR of the given delay and automatically resets the channel
 * once the ISR has been called
 * 
 */
void timer_ms_isr_ch1(void);

/**
 * @brief ISR for channel 2. Calls the ISR of the given delay and automatically resets the channel
 * once the ISR has been called
 * 
 */
void timer_ms_isr_ch2(void);

/**
 * @brief Returns true or false depending on whether channel 1 is free
 * 
 * @return uint8_t 1 if channel 1 is free else 0
 */
uint8_t timer_ms_ch1_is_free(void);

/**
 * @brief Returns true or false depending on whether channel 1 is free
 * 
 * @return uint8_t 1 if channel 1 is free else 0
 */
uint8_t timer_ms_ch2_is_free(void);

/**
 * @brief Determines whether a given handle has a current delay set or not
 * 
 * @param handle The handle to check
 * @return uint8_t 1 if the handle does have a delay set else 0
 */
uint8_t timer_ms_handle_in_use(TimerMsHandle* handle);

/**
 * @brief Prints the current status of the channel that is running the handles delay.
 * If the handle is not being used by a channel, prints the current timer count and
 * a message stating the handle is not in use
 * 
 * @param handle The handle to check the status of
 */
void timer_ms_print_handle_status(TimerMsHandle* handle);

#endif // TIMER_MS_H