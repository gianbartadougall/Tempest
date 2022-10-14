/**
 * @file piezzo_buzzer.c
 * @author Gian Barta-Dougall
 * @brief Peripheral driver for piezzo buzzer
 * @version 0.1
 * @date 2022-06-25
 *
 * @copyright Copyright (c) 2022
 *
 */
/* Public Includes */

/* Private Includes */
#include "piezo_buzzer.h"
#include "hardware_config.h"
#include "debug_log.h"

/* STM32 Includes */

/* Private #defines */

// #define PIEZO_PIN_RAW 6
// #define PIEZO_PIN (PIEZO_PIN_RAW * 2)
// #define PIEZO_PORT GPIOA
// #define PIEZO_PORT_CLK_POS (0x01 << RCC_AHB2ENR_GPIOAEN)
// #define PIEZO_IRQn EXTI9_5_IRQn

// #define PIEZO_TIMER TIM16
// #define PIEZO_TIMER_IRQ_NUMBER TIM16_IRQn // Interrupt handler for Timer 2
// #define PIEZO_TIMER_CLK_ENABLE() __HAL_RCC_TIM16_CLK_ENABLE() // Clock enable function for Timer 2
// #define PIEZO_TIMER_FREQUENCY ((uint32_t) 100000) // Timer Frequency set to 100 khz
// #define PIEZO_TIMER_PRESCALER ((uint16_t) (SystemCoreClock / PIEZO_TIMER_FREQUENCY) - 1) // Set prescaler for Timer
// to run at desired frequency #define PIEZO_TIMER_MAX_COUNT ((uint16_t) 65535)// Max count for 16 bit timer #define
// #define PIEZO_TIMER_IRQn
// // TIM1_UP_TIM16_IRQn
#define PIEZO_TIMER                              HC_PIEZO_BUZZER_TIMER
#define PIEZO_TIMER_ARR_VALUE(frequency)         ((HC_PIEZO_BUZZER_TIMER_FREQUENCY / frequency) - 1)
#define PIEZO_TIMER_DUTY_CYCLE(dutyCyclePercent) ((HC_PIEZO_BUZZER_TIMER->ARR * dutyCyclePercent) / 100)

#define NUM_SOUNDS 6
/* Defines for sounds */

const PiezoNote endNote = {
    .frequency = 0,
    .dutyCycle = 0,
};

const PiezoNote aMajor = {
    .frequency = 200,
    .dutyCycle = 50,
};

const PiezoNote aMinor = {
    .frequency = 500,
    .dutyCycle = 50,
};

PiezoNote sounds[NUM_SOUNDS] = {aMajor, aMinor, aMajor, aMinor, aMajor, endNote};

/* Variable Declarations */
uint8_t pbIndex = 0;

/* Function prototypes */
void piezo_buzzer_set_note(void);
void piezo_buzzer_enable(void);
void piezo_buzzer_disable(void);

void piezo_buzzer_play_sound(uint8_t startIndex) {

    // Check if the given index is an endNote
    if (sounds[startIndex].frequency == 0) {
        return;
    }

    pbIndex = startIndex;

    // Set the next note to be played by the buzzer
    piezo_buzzer_set_note();

    // Enable the buzzer
    piezo_buzzer_enable();
}

void piezo_buzzer_isr(void) {

    pbIndex++;

    if (pbIndex > NUM_SOUNDS || sounds[pbIndex].frequency == 0) {
        piezo_buzzer_disable();
        return;
    }

    // Set the next note to be played by the buzzer
    piezo_buzzer_set_note();
}

void piezo_buzzer_set_note(void) {

    // Set the frequency of the PWM output (max timer count)
    PIEZO_TIMER->ARR = PIEZO_TIMER_ARR_VALUE(sounds[pbIndex].frequency);

    // Set the duty cycle of the PWM output (output compare number)
    PIEZO_TIMER->CCR1 = PIEZO_TIMER_DUTY_CYCLE(sounds[pbIndex].dutyCycle);
}

void piezo_buzzer_disable(void) {
    PIEZO_TIMER->DIER &= ~(TIM_DIER_UIE);    // Disable interrupts
    PIEZO_TIMER->CR1 &= ~(TIM_CR1_CEN);      // Disable the timer
    PIEZO_TIMER->CCMR1 &= ~(TIM_CCMR1_OC1M); // Reset PWM mode
    PIEZO_TIMER->CCMR1 |= TIM_CCMR1_OC1M_3;  // Force PWM output low
}

void piezo_buzzer_enable(void) {
    PIEZO_TIMER->DIER |= TIM_DIER_UIE;
    PIEZO_TIMER->CR1 |= TIM_CR1_CEN;                             // Enable the counter
    PIEZO_TIMER->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2); // Set mode to PWM mode 1
}