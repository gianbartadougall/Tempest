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
#define PIEZO_TIMER                              HC_PIEZO_BUZZER_TIMER
#define PIEZO_TIMER_ARR_VALUE(frequency)         ((HC_PIEZO_BUZZER_TIMER_FREQUENCY / frequency) - 1)
#define PIEZO_TIMER_DUTY_CYCLE(dutyCyclePercent) ((HC_PIEZO_BUZZER_TIMER->ARR * dutyCyclePercent) / 100)

#define NUM_SOUNDS 52
/* Defines for sounds */

const PiezoNote endNote = {
    .frequency = 0,
    .dutyCycle = 0,
};

const PiezoNote aMajor = {
    .frequency = 600,
    .dutyCycle = 90,
};

const PiezoNote aMinor = {
    .frequency = 1500,
    .dutyCycle = 90,
};

const PiezoNote bMinor = {
    .frequency = 200,
    .dutyCycle = 90,
};

PiezoNote sounds[NUM_SOUNDS] = {aMinor, aMinor, aMinor, aMinor, aMinor, aMinor, aMinor, aMinor, aMinor,  aMinor, aMinor,
                                aMinor, aMinor, aMinor, aMinor, aMinor, aMinor, aMinor, aMinor, aMinor,  aMinor, aMinor,
                                aMinor, aMinor, aMinor, aMinor, aMinor, aMinor, aMinor, aMinor, aMinor,  aMinor, aMinor,
                                aMinor, aMinor, aMinor, aMajor, aMajor, aMajor, aMajor, aMajor, endNote, bMinor, bMinor,
                                bMinor, bMinor, bMinor, bMinor, bMinor, bMinor, bMinor, endNote};

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