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
#include "debug_log.h"

/* STM32 Includes */

/* Private #defines */

#define PIEZO_PIN_RAW 6
#define PIEZO_PIN (PIEZO_PIN_RAW * 2)
#define PIEZO_PORT GPIOA
#define PIEZO_PORT_CLK_POS (0x01 << RCC_AHB2ENR_GPIOAEN)
#define PIEZO_IRQn EXTI9_5_IRQn

#define PIEZO_TIMER TIM16
#define PIEZO_TIMER_IRQ_NUMBER TIM16_IRQn // Interrupt handler for Timer 2
#define PIEZO_TIMER_CLK_ENABLE() __HAL_RCC_TIM16_CLK_ENABLE() // Clock enable function for Timer 2
#define PIEZO_TIMER_FREQUENCY ((uint32_t) 100000) // Timer Frequency set to 100 khz
#define PIEZO_TIMER_PRESCALER ((uint16_t) (SystemCoreClock / PIEZO_TIMER_FREQUENCY) - 1) // Set prescaler for Timer to run at desired frequency
#define PIEZO_TIMER_MAX_COUNT ((uint16_t) 65535)// Max count for 16 bit timer
#define PIEZO_TIMER_ARR_VALUE(frequency) ((PIEZO_TIMER_FREQUENCY / frequency) - 1) // Pulse width in us
#define PIEZO_TIMER_DUTY_CYCLE(dutyCyclePercent) ((PIEZO_TIMER->ARR * dutyCyclePercent) / 100)
#define PIEZO_TIMER_IRQn TIM1_UP_TIM16_IRQn

/* Defines for sounds */


/* Variable Declarations */

PiezoSound sound1;
PiezoSound sound2;

PiezoSound currentSound;

/* Function prototypes */
void piezo_buzzer_hardware_init(void);
void piezo_buzzer_init_sounds(void);

void piezo_buzzer_init(void) {

    // Initialise piezo buzzer hardware
    piezo_buzzer_hardware_init();

    // Initialise piezo buzzer sounds
    piezo_buzzer_init_sounds();
    
}

void piezo_buzzer_hardware_init(void) {
    // Initialise GPIO pin
    PIEZO_PORT->MODER   &= ~(0x03 << PIEZO_PIN); // Reset mode pin
    PIEZO_PORT->MODER   |= (0x02 << PIEZO_PIN); // Set pin to alternate function
    PIEZO_PORT->OSPEEDR |= (0x03 << PIEZO_PIN); // Set pin to very high speed
    PIEZO_PORT->PUPDR   &= ~(0x03 << PIEZO_PIN); // Set pin to no pull up/down
    PIEZO_PORT->OTYPER  &= ~(0x01 << PIEZO_PIN_RAW); // Set pin to push-pull

    // Configure alternate function
    PIEZO_PORT->AFR[0] &= ~(0x0F << (PIEZO_PIN_RAW * 4)); // Reset alternate function
    PIEZO_PORT->AFR[0] |= (0x0E << (PIEZO_PIN_RAW * 4)); // Set alternate function to AF14

    // Enable GPIO Clock
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Initialise timer */

    // Enable timer clock
    PIEZO_TIMER_CLK_ENABLE();

    PIEZO_TIMER->PSC = PIEZO_TIMER_PRESCALER;
    PIEZO_TIMER->CR1 &= ~TIM_CR1_DIR; // Set timer to count upwards
    
    PIEZO_TIMER->ARR = PIEZO_TIMER_ARR_VALUE(20000);
    PIEZO_TIMER->CCR1 = PIEZO_TIMER_DUTY_CYCLE(50);

    // Disable all outputs
    PIEZO_TIMER->CCER = 0x00;

    // Enable capture compare output
    PIEZO_TIMER->CCER |= TIM_CCER_CC1E;

    PIEZO_TIMER->CCMR1 &= ~(0x03); // Set timer to output
    PIEZO_TIMER->CCMR1 &= ~(TIM_CCMR1_OC1M); // Reset timer output
	PIEZO_TIMER->CCMR1 |= (0x4 << 4); // Force pin low by default 
	PIEZO_TIMER->CCMR1 |= (TIM_CCMR1_OC1PE); // Enable output preload bit for channel 2 

    // Set CC1 channel to output
    PIEZO_TIMER->BDTR |= TIM_BDTR_MOE;

    HAL_NVIC_SetPriority(PIEZO_TIMER_IRQn, 10, 0); // Set interrupt priority
	HAL_NVIC_EnableIRQ(PIEZO_TIMER_IRQn); // Enable interrupt

    // Enable interrupts
    PIEZO_TIMER->DIER = 0x00; // Disable all interrupts
    PIEZO_TIMER->DIER |= TIM_DIER_UIE; 
}

void piezo_buzzer_init_sounds(void) {
    sound1.numMaxPlays = 5;
    sound1.index = 0;
    sound1.frequencies[0] = 20;
    sound1.frequencies[1] = 100;
    sound1.frequencies[2] = 20;
    sound1.frequencies[3] = 100;
    sound1.frequencies[4] = 100;
    sound1.dutyCycles[0] = 5;
    sound1.dutyCycles[1] = 0;
    sound1.dutyCycles[2] = 5;
    sound1.dutyCycles[3] = 0;
    sound1.dutyCycles[4] = 0;

    sound2.numMaxPlays = 5;
    sound2.index = 0;
    sound2.frequencies[0] = 1000;
    sound2.frequencies[1] = 100;
    sound2.frequencies[2] = 1000;
    sound2.frequencies[3] = 100;
    sound2.frequencies[4] = 1000;
    sound2.dutyCycles[0] = 50;
    sound2.dutyCycles[1] = 0;
    sound2.dutyCycles[2] = 50;
    sound2.dutyCycles[3] = 0;
    sound2.dutyCycles[4] = 0;
}

void piezo_buzzer_play_sound(PiezoSound sound) {

    // Set the sound to play and reset the index
    currentSound = sound;
    currentSound.index = 0;

    // Clear all current interrupts

    PIEZO_TIMER->ARR = PIEZO_TIMER_ARR_VALUE(currentSound.frequencies[currentSound.index]);
    PIEZO_TIMER->CCR1 = PIEZO_TIMER_DUTY_CYCLE(currentSound.dutyCycles[currentSound.index]);

    // Set timer to pwm mode
    PIEZO_TIMER->CCMR1 &= ~(TIM_CCMR1_OC1M); 	// Clear OC1M (Channel 1) 
	PIEZO_TIMER->CCMR1 |= (0x6 << 4); 		// Enable PWM Mode 1, upcounting, on Channel 2 

    // Set count to 0 and enable the counter
    PIEZO_TIMER->CNT = 0;
    PIEZO_TIMER->CR1 |= TIM_CR1_CEN;
}

void piezo_buzzer_isr(void) {

    currentSound.index++;
    
    if (currentSound.index == currentSound.numMaxPlays) {
        // Disable the counter
        PIEZO_TIMER->CR1 &= ~TIM_CR1_CEN;

        // Force output to go low
        PIEZO_TIMER->CCMR1 &= ~TIM_CCMR1_OC1M;
        PIEZO_TIMER->CCMR1 |= 0x04 << 4;

        return;
    }
    // Reset timer count to 0
    PIEZO_TIMER->CNT = 0;

    // Update timer settings
    PIEZO_TIMER->ARR = PIEZO_TIMER_ARR_VALUE(currentSound.frequencies[currentSound.index]);
    PIEZO_TIMER->CCR1 = PIEZO_TIMER_DUTY_CYCLE(currentSound.dutyCycles[currentSound.index]);

}