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
#include "debug_log.h"
#include "interrupts_config.h"

/* STM32 Includes */

/* Private #defines */
#define PUSH_BUTTONS 2

/* CONFIG SECTION */
#define PB_TYPE_GENERIC 0x01 // Determines if state is high or low through polling
#define PB_TYPE_ISR (0x02 | PB_TYPE_GENERIC) // Adds ISR functionality to timer
#define PB_TYPE_TIMER (0x04 | PB_TYPE_ISR | PB_TYPE_GENERIC) // Adds functionality to determine time the pushbutton has been pressed for

#define PB0_TYPE PB_TYPE_TIMER
#define PB1_TYPE PB_TYPE_TIMER

// Compile TIMER definitions if timer is a feature
#if ((PB0_TYPE | PB1_TYPE) & PB_TYPE_TIMER)

#define PB_TIMER TIM2
#define PB_TIMER_FREQUENCY 1000 // Frequency set to 1kHz
#define PB_TIMER_MAX_COUNT 65534
#define PB_TIMER_CLK_ENABLE() __HAL_RCC_TIM2_CLK_ENABLE()
#define PB_TIMER_IRQn TIM2_IRQn

#define PB0_FLAG_TIME 3000
#define PB1_FLAG_TIME 3000

// Lets ISR know whether the rising edge has been triggered or not
#define UNDEFINED_TRIGGER_COUNT 65535

// Compilation check to ensure this equality is met
#if (PB_TIMER_MAX_COUNT > UNDEFINED_TRIGGER_COUNT)
    #error UNDEFINED_TRIGGER_COUNT must be greater than TIMER_MAX_COUNT
#endif

#endif

/* Variable Declarations */
uint32_t pbPins[PUSH_BUTTONS] = {0, 3};
GPIO_TypeDef* pbPorts[PUSH_BUTTONS] = {GPIOA, GPIOA};
IRQn_Type extiLines[PUSH_BUTTONS] = {EXTI0_IRQn, EXTI2_IRQn};
uint8_t pbTypes[PUSH_BUTTONS] = {PB0_TYPE, PB1_TYPE};

uint32_t pb0StartTime = 0;
uint32_t pb0TimePressed = 0;
uint8_t pb0FlagTimeMissed = 0;

/* Function prototypes */
void pb_hardware_init(uint8_t index);

void pb_init(void) {

    // Initialise pushbutton hardware
    for (int i = 0; i < PUSH_BUTTONS; i++) {

        // pb_hardware_init pin hardware
        pb_hardware_init(i);

        // Check if ISR functionality is to be added
        if ((pbTypes[i] & PB_TYPE_ISR) != 0) {

            // Clear trigger line
            SYSCFG->EXTICR[pbPins[i] / 4] &= ~(0x07 << (4 * pbPins[i])); // Clear trigger line

            // Set trigger line to correct pin. Note PA lines should be set to 0x000 thus the reset above sets
            // the trigger line to PA by default
            if (pbPorts[i] == GPIOB) {
                SYSCFG->EXTICR[pbPins[i] / 4] |= (0x01 << (4 * pbPins[i])); // Set trigger line for given pin
            }
            
            EXTI->RTSR1 |= (0x01 << pbPins[i]); // Enable trigger on rising edge
            EXTI->FTSR1 |= (0x01 << pbPins[i]); // Enable interrupt on falling edge
            EXTI->IMR1  |= (0x01 << pbPins[i]); // Enabe external interrupt for EXTI line

            // Enable Clock
            RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

            HAL_NVIC_SetPriority(extiLines[i], EXTI0_ISR_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(extiLines[i]);
        }
  
    }

    // Enable timer if required
    #ifdef PB_TIMER

        // Enable the timer clock
        PB_TIMER_CLK_ENABLE();

        // Set the prescaler
        PB_TIMER->PSC = (SystemCoreClock / PB_TIMER_FREQUENCY) - 1; // Set frequency to 2Khz
        PB_TIMER->ARR = PB_TIMER_MAX_COUNT;
    
        // Enable timer interrupts on CH1 and CH4
        PB_TIMER->DIER |= TIM_DIER_TIE | TIM_DIER_CC1IE | TIM_DIER_CC2IE | TIM_DIER_UIE;

        /* Configure and enable output compare on CH1 and CH2 */
        PB_TIMER->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_CC2S); // Set CH1 and CH2 to output mode
        PB_TIMER->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M); // Set CH1 and CH2 output compare to Frozen

        PB_TIMER->CCER |= (TIM_CCER_CC2E | TIM_CCER_CC1E); // Enable capture compare on CH1 and CH2

        /* Set output compare values */
        PB_TIMER->CCR1 = PB0_FLAG_TIME; // Set output compare on CH1
        PB_TIMER->CCR2 = PB1_FLAG_TIME; // Set output compare on CH2

        PB_TIMER->CR1 &= ~(0x01 << 4); // Set counter direction to up counting

        // Enable interrupts on timer
        HAL_NVIC_SetPriority(PB_TIMER_IRQn, TIM2_ISR_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(PB_TIMER_IRQn);

    #endif
}

void pb_hardware_init(uint8_t index) {

    // Set pushbutton to generic input
    pbPorts[index]->MODER  &= ~(0x03 << (pbPins[index] * 2)); // Reset mode
    pbPorts[index]->MODER  |= (0x02 << (pbPins[index] * 2)); // Set mode to alternate function
    pbPorts[index]->OSPEEDR &= ~(0x03 << (pbPins[index] * 2)); // Set pin to low speed
    pbPorts[index]->PUPDR  &= ~(0x03 << (pbPins[index] * 2)); // Set pin to no pull up/down
    pbPorts[index]->OTYPER &= ~(0x01 << pbPins[index]); // Set pin to push-pull

    // Enable clock for the GPIO pin
    if (pbPorts[index] == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (pbPorts[index] == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
}

void pb_start_timer(void) {

    // Set the capture compare values to undefined to ensure code in
    // ISR for rising edge is always executed before code for falling
    // edge 
    PB_TIMER->CCR1 = UNDEFINED_TRIGGER_COUNT;
    PB_TIMER->CCR2 = UNDEFINED_TRIGGER_COUNT;

    PB_TIMER->CNT = 0; // Reset the counter to 0
    PB_TIMER->CR1 |= 0x01; // Enable the counter
}

void pb_stop_timer(void) {
    PB_TIMER->CR1 &= ~(0x01); // Disable counter
}

void pb_0_isr(void) {

    // Run timer ISR code if the timer has been enabled
    if ((PB_TIMER->CR1 & TIM_CR1_CEN) == TIM_CR1_CEN) {
        
        /* Set capture compare 1 value if ISR was a triggered by a rising edge. 
            Calculate the time pb0 was pressed for if ISR was trigger by a 
            falling edge 
        */
        if ((pbPorts[0]->IDR & (0x01 << pbPins[0])) != 0) {
            
            pb0StartTime = PB_TIMER->CNT; // Store the current timer count

            // Set the CCR register for CH1 to trigger in required time
            uint32_t difference = PB_TIMER->ARR - PB_TIMER->CNT;
            
            if (PB0_FLAG_TIME > difference) {
                PB_TIMER->CCR1 = PB0_FLAG_TIME - difference;
            } else {
                PB_TIMER->CCR1 = PB_TIMER->CNT + PB0_FLAG_TIME;
            }
        } else {
            
            // If CCR1 is undefined exit as the rising edge has not been triggered yet
            // for this cycle
            if (PB_TIMER->CCR1 == UNDEFINED_TRIGGER_COUNT) {
                return;
            }

            /* Calculate the length of time the button has been down for. Set the time
                flag if the calculate time doesn't meet minimum requirement
            */

            uint32_t timerCount = PB_TIMER->CNT; // Store current count
            uint32_t pb0TimePressed;

            // Check if timer has reset
            if (timerCount < PB_TIMER->CNT) {
                pb0TimePressed = (PB_TIMER_MAX_COUNT - pb0StartTime) + PB_TIMER->CNT; 
            } else {
                pb0TimePressed = PB_TIMER->CNT - pb0StartTime;
            }

            if (pb0TimePressed < PB0_FLAG_TIME) {
                pb0FlagTimeMissed = 1; // Set flag to indicate flag time was missed
                
                // Call TIM ISR. TIM ISR priority is greater than the EXTI ISR priortiy
                // thus the system will be notified of this before the flag is reset
                // in on the following line
                PB_TIMER->EGR |= (TIM_EGR_CC1G); // Call TIM ISR

                pb0FlagTimeMissed = 0; // Reset flag for new cycle
            }
        }
    }
}

uint8_t pb_get_state(uint8_t pushButton) {
    
    if (pushButton > (PUSH_BUTTONS - 1)) {
        return 255;
    }
    
    uint32_t pinIndex;

    switch (pushButton) {
        case 0:
            pinIndex = 0x01 << pbPins[0];
            return ((pbPorts[0]->IDR & pinIndex) == pinIndex ? GPIO_PIN_SET : GPIO_PIN_RESET);
        case 1:
            pinIndex = 0x01 << pbPins[1];
            return ((pbPorts[1]->IDR & pinIndex) == pinIndex ? GPIO_PIN_SET : GPIO_PIN_RESET);
        default:
            return 255;
    }
}