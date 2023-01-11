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
#include "log.h"

/* STM32 Includes */

/* Private #defines */
#define PIEZO_TIMER                              HC_PIEZO_BUZZER_TIMER
#define PIEZO_TIMER_ARR_VALUE(frequency)         ((HC_PIEZO_BUZZER_TIMER_FREQUENCY / frequency) - 1)
#define PIEZO_TIMER_DUTY_CYCLE(dutyCyclePercent) ((HC_PIEZO_BUZZER_TIMER->ARR * dutyCyclePercent) / 100)

#define NUM_SOUNDS (4 * 21)
/* Defines for sounds */

const PiezoNote endNote = {
    .frequency = 0,
    .dutyCycle = 0,
};

const PiezoNote a = {
    .frequency = 900,
    .dutyCycle = 50,
};

const PiezoNote b = {
    .frequency = 600,
    .dutyCycle = 90,
};

const PiezoNote c = {
    .frequency = 2000,
    .dutyCycle = 90,
};

const PiezoNote d = {
    .frequency = 200,
    .dutyCycle = 90,
};

// Random sounds
#define SOUND_0 a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, endNote
#define SOUND_1 b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, endNote
#define SOUND_2 c, c, c, c, c, c, c, c, c, c, c, c, c, c, c, c, c, c, c, c, endNote
#define SOUND_3 d, d, d, d, d, d, d, d, d, d, d, d, d, d, d, d, d, d, d, d, endNote

PiezoNote sounds[NUM_SOUNDS] = {SOUND_0, SOUND_1, SOUND_2, SOUND_3};

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
    piezo_buzzer_enable();
    piezo_buzzer_disable();

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
    PIEZO_TIMER->DIER |= TIM_DIER_UIE;                           // Enable interrupts
    PIEZO_TIMER->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2); // Set mode to PWM mode 1
    PIEZO_TIMER->CR1 |= TIM_CR1_CEN;                             // Enable the counter
}