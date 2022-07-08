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
#include "timer_ms.h"
#include "debug_log.h"
#include "interrupts_config.h"

/* STM32 Includes */

/* Private #defines */
#define PUSH_BUTTONS 2 // The number of pushbuttons being used

/* Variable Declarations */

// Timer tasks used for debouncing pushbuttons
TimerMsTask Pb0DebounceTask;
TimerMsTask Pb1DebounceTask;

typedef struct PushButton {
    uint32_t pin;
    GPIO_TypeDef* port;
    IRQn_Type extiLine;        
} PushButton;

// List to hold the pushbutton structs
PushButton pushbuttons[PUSH_BUTTONS];

// List to hold debounce flag for each pushbutton. The debounce
// flag is used by the ISRs to determine whether the button is
// still bouncing or not
uint8_t pbDebouncedFlag[PUSH_BUTTONS];

// List of function pointers. These are called by the ISR depending on the mode
// the edge that triggered the ISR
void (*mode_0_re_callbacks[PUSH_BUTTONS]) (void); // Mode 0 rising edge callback
void (*mode_0_fe_callbacks[PUSH_BUTTONS]) (void); // Mode 0 falling edge callback
void (*mode_1_re_callbacks[PUSH_BUTTONS]) (void); // Mode 1 rising edge callback
void (*mode_1_fe_callbacks[PUSH_BUTTONS]) (void); // Mode 1 falling edge callback
void (*mode_2_re_callbacks[PUSH_BUTTONS]) (void); // Mode 2 rising edge callback
void (*mode_2_fe_callbacks[PUSH_BUTTONS]) (void); // Mode 2 falling edge callback

// Default mode for pushbuttons
uint8_t pbMode = PB_MODE_DEFAULT;

/* Private Function prototypes */
void pb_hardware_init(uint8_t index);
void pb0_reset_timer(void);
void pb0_debounce_task_isr(void);
void pb1_debounce_task_isr(void);

/* Public Functions */

void pb_init(void) {

    // Initialise clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure pushbutton parameters */
    pushbuttons[0].pin = 0;
    pushbuttons[0].port = GPIOA;
    pushbuttons[0].extiLine = EXTI0_IRQn;

    pushbuttons[1].pin = 3;
    pushbuttons[1].port = GPIOA;
    pushbuttons[1].extiLine = EXTI3_IRQn;

    // Set all debounce flags
    for (uint8_t i = 0; i < PUSH_BUTTONS; i++) {
        pbDebouncedFlag[i] = 1;
    }

    // Initialise pushbutton hardware
    for (uint8_t i = 0; i < PUSH_BUTTONS; i++) {

        // Set pushbutton to generic input
        pushbuttons[i].port->MODER  &= ~(0x03 << (pushbuttons[i].pin * 2)); // Reset mode to input
        pushbuttons[i].port->OSPEEDR &= ~(0x03 << (pushbuttons[i].pin * 2)); // Set pin to low speed
        pushbuttons[i].port->PUPDR  &= ~(0x03 << (pushbuttons[i].pin * 2)); // Set pin to no pull up/down
        pushbuttons[i].port->OTYPER &= ~(0x01 << pushbuttons[i].pin); // Set pin to push-pull

        /* Enable Interrupt for pushbutton */

        // Clear trigger line
        SYSCFG->EXTICR[pushbuttons[i].pin / 4] &= ~(0x07 << (4 * pushbuttons[i].pin)); // Clear trigger line

        // Set trigger line to correct pin. Note PA lines should be set to 0x000 thus the reset above sets
        // the trigger line to PA by default
        if (pushbuttons[i].port == GPIOB) {
            SYSCFG->EXTICR[pushbuttons[i].pin / 4] |= (0x01 << (4 * pushbuttons[i].pin)); // Set trigger line for given pin
        }
        
        EXTI->RTSR1 |= (0x01 << pushbuttons[i].pin); // Enable trigger on rising edge
        EXTI->FTSR1 |= (0x01 << pushbuttons[i].pin); // Enable interrupt on falling edge
        EXTI->IMR1  |= (0x01 << pushbuttons[i].pin); // Enabe external interrupt for EXTI line

        // Enable Clock
        RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

        HAL_NVIC_SetPriority(pushbuttons[i].extiLine, EXTI0_ISR_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(pushbuttons[i].extiLine);
    }

    // Initialise the Timer Tasks used for debouncing

	void (*pb0DebounceISRs[1]) (void) = {&pb0_debounce_task_isr};
	uint16_t pb0DebounceDelays[1] = {10}; // Wait 5 milliseconds before running ISR
	timer_ms_init_task(
		&Pb0DebounceTask, pb0DebounceISRs, pb0DebounceDelays, 1, TIMER_MS_TASK_PAUSE, TASK_MS_PRIORITY_2, TIMER_MS_TASK_FINISH
	);

	void (*pb1DebounceISRs[1]) (void) = {&pb1_debounce_task_isr};
	uint16_t pb1DebounceDelays[1] = {10}; // Wait 5 milliseconds before running ISR
	timer_ms_init_task(
		&Pb1DebounceTask, pb1DebounceISRs, pb1DebounceDelays, 1, TIMER_MS_TASK_PAUSE, TASK_MS_PRIORITY_2, TIMER_MS_TASK_FINISH
	);
    
}


void pb_set_mode(uint8_t mode) {
    pbMode = mode;
}

void pb_set_callbacks(uint8_t x, uint8_t mode, void (*re_callback)(void), void (*fe_callback)(void)) {

    switch (mode) {
        case PB_MODE_0:
            mode_0_re_callbacks[x] = re_callback;
            mode_0_fe_callbacks[x] = fe_callback;
            break;
        
        case PB_MODE_1:
            mode_1_re_callbacks[x] = re_callback;
            mode_1_fe_callbacks[x] = fe_callback;
            break;

        case PB_MODE_2:
            mode_2_re_callbacks[x] = re_callback;
            mode_2_fe_callbacks[x] = fe_callback;
            break;
        
        default:
            break;
    }
}

void pb_isr(uint8_t x) {

    // Determine whether the ISR was triggered by a rising or falling edge
    if ((pushbuttons[x].port->IDR & (0x01 << pushbuttons[x].pin)) != 0) {

        // Cancel the debouncing task. If not enough time has passed, this 
        // ensures that timer task will need to be restarted on the next
        // falling edge. If that task has already finished, cancelling does
        // nothing
        switch (x) {
            case PUSH_BUTTON_0:
                timer_ms_cancel_task(&Pb0DebounceTask);
                break;
            case PUSH_BUTTON_1:
                timer_ms_cancel_task(&Pb1DebounceTask);
                break;
            default:
                break;
        }

        // Run rising edge code if debounce flag is set
        if (pbDebouncedFlag[x] == 1) {
            
            // The ISR will perform a different action depending on the current mode
            switch (pbMode) {
                case PB_MODE_0: // Call rising edge callback function mode 0
                    (*mode_0_re_callbacks[x]) ();
                    break;
                case PB_MODE_1: // Call rising edge callback function mode 1
                    (*mode_1_re_callbacks[x]) ();
                    break;
                case PB_MODE_2: // Call rising edge callback function mode 1
                    (*mode_2_re_callbacks[x]) ();
                default: // Do nothing by default
                    break;
            }
        }

    } else {
        
        // If the debounce flag is high, run the falling edge code. This ensures
        // the falling edge code is not run multiple times
        if (pbDebouncedFlag[x] == 1) {

            switch (pbMode) {
                case PB_MODE_0: // Call falling edge callback function mode 0
                    (*mode_0_fe_callbacks[x]) ();
                    break;
                case PB_MODE_1: // Call falling edge callback function mode 1
                    (*mode_1_fe_callbacks[x]) ();
                    break;
                case PB_MODE_2: // Call rising edge callback function mode 1
                    (*mode_2_fe_callbacks[x]) ();
                default: // Do nothing by default
                    break;
            }
        }

        // Clear the debounce flag
        pbDebouncedFlag[x] = 0;

        switch (x) {
            case PUSH_BUTTON_0:
                // Start debounce task if it is not already running
                if (timer_ms_task_is_in_queue(&Pb0DebounceTask) == 0) {
                    timer_ms_add_task(&Pb0DebounceTask);
                }
                break;
            case PUSH_BUTTON_1:
                if (timer_ms_task_is_in_queue(&Pb1DebounceTask) == 0) {
                    timer_ms_add_task(&Pb1DebounceTask);
                }
                break;
            default:
                break;
        }
    }
}

uint8_t pb_get_state(uint8_t x) {
    
    // Check the given pushbutton is valid
    if (x > (PUSH_BUTTONS - 1)) {
        return 255;
    }
    
    // uint32_t pinIndex = (0x01 << pushbuttons[x].pin);
    return (pushbuttons[x].port->IDR & (0x01 << pushbuttons[x].pin)) == 0 ? 0 : 1; 
}

/* Private Functions */

/**
 * @brief This function is automatically called by the debounce timer and
 * will set the debounce flag to allow the next rising edge to pass
 */
void pb0_debounce_task_isr(void) {
    pbDebouncedFlag[PUSH_BUTTON_0] = 1;
}

/**
 * @brief This function is automatically called by the debounce timer and
 * will set the debounce flag to allow the next rising edge to pass
 */
void pb1_debounce_task_isr(void) {
    pbDebouncedFlag[PUSH_BUTTON_1] = 1;
}