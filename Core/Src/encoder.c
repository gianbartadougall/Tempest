/**
 * @file encoder.c
 * @author Gian Barta-Dougall
 * @brief Peripheral driver for a rotary encoder
 * @version 0.1
 * @date 2022-06-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/* Public Includes */

/* Private Includes */
#include "encoder.h"
#include "debug_log.h"
#include "comparator.h"

/* STM32 Includes */

/* Private #defines */
#define ENCODER_PIN_RAW 8
#define ENCODER_PIN (ENCODER_PIN_RAW * 2)
#define ENCODER_PORT GPIOA
#define ENCODER_PORT_CLK_POS (0x01 << RCC_AHB2ENR_GPIOAEN)
#define ENCODER_IRQn EXTI9_5_IRQn

#define ENCODER_MICROMETER 1000000
#define ENCODER_MILLIMETER 1000
#define ENCODER_CENTIMETER 100
#define ENCODER_METER 1
#define DISTANCE_UNIT ENCODER_MICROMETER

#define ENCODER_CIRCUMFERENCE 75398 // micrometers
#define ENCODER_NUM_GEAR_TEETH 8
#define ENCODER_DISTANCE_PER_TOOTH ((uint32_t) (ENCODER_CIRCUMFERENCE / ENCODER_NUM_GEAR_TEETH))

#define ENCODER_TIMER TIM1

// The maximum distance the object needs to travel in micrometers
#define ENOCDER_OBJECT_DISTANCE 1450000 // (1.45m) // 867054 - 46 ISR
#define ENCODER_MINIMUM_DISTANCE 0
#define ENCODER_MAXIMUM_DISTANCE (((uint32_t) (ENOCDER_OBJECT_DISTANCE / ENCODER_CIRCUMFERENCE)) * ENCODER_DISTANCE_PER_TOOTH * ENCODER_NUM_GEAR_TEETH)

/* Variable Declarations */
uint32_t distanceTravelled = 0;
uint32_t teethPassed       = 0;
uint8_t encoderStatus      = 0;
uint16_t isrCount    = 0;
uint16_t absISRCount = 0;
uint32_t time1 = 0;
uint32_t time2 = 0;
uint16_t maximumCount = 88;

uint16_t ccr1Temp = 0;
uint16_t ccr2Temp = 0;

/* Function prototypes */
void encoder_timer_init(void);

void encoder_hardware_init(void) {

    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Set encoder pin to alternate function connected to the input of TIM1
    ENCODER_PORT->MODER  &= ~(0x03 << ENCODER_PIN); // Reset mode
    ENCODER_PORT->MODER  |= (0x02 << ENCODER_PIN); // Set mode to alternate function
    
    ENCODER_PORT->OSPEEDR &= ~(0x03 << ENCODER_PIN); // Set pin to low speed
    
    ENCODER_PORT->PUPDR  &= ~(0x03 << ENCODER_PIN); // Clear PUPDR for no pull up/down
    
    ENCODER_PORT->OTYPER &= ~(0x01 << ENCODER_PIN);
    
    ENCODER_PORT->AFR[1] &= ~(0x0F); // Reset alternate function
    ENCODER_PORT->AFR[1] |= (0x01); // Set alternate function to AF1

}

void encoder_init(void) {

    // Initialise internal comparator
    // comparator_init();
    // Initialise input pin
    encoder_hardware_init();

    // Initialise timer such that it will count on the rising edge of the output
    // of the comparator
    encoder_timer_init();
}

void encoder_timer_init(void) {

    // Configure channel 1 to be input capture 

    // Enable the clock for the timer
    __HAL_RCC_TIM1_CLK_ENABLE();    

    // Set the sampling rate. (I'm not 100% sure if this is required. I
    // tested changing it and it didn't seem to affect the output)
    ENCODER_TIMER->PSC = ((SystemCoreClock / 1000000) - 1); // Set sampling rate

    // Set the maximum limit for the counter
    ENCODER_TIMER->ARR = 10000;

    // Set counter to count upwards
    ENCODER_TIMER->CR1 &= ~(0x01 << 4);

    ENCODER_TIMER->CR2 &= ~(0x01 << 7); // Set CH1 to timer input 1

    // Set TIM1 to input and map TIM_CH1 GPIO pin to trigger input 1 (TI1)
    ENCODER_TIMER->CCMR1 &= ~(0x03 << 0); // Reset capture compare
    ENCODER_TIMER->CCMR1 |= (0x01 << 0); // Set capture compare to input (IC1 mapped to TI1)
    
    // Configure slave mode control
    ENCODER_TIMER->SMCR &= ~(0x07 << 4); // Reset trigger selection
    ENCODER_TIMER->SMCR |= (0x05 << 4); // Set trigger to Filtered Timer Input 1 (TI1FP1)
    ENCODER_TIMER->SMCR &= ~((0x01 << 16) | 0x07); // Reset slave mode selection
    ENCODER_TIMER->SMCR |= 0x07; // Set rising edge of selected trigger to clock the counter
    
    /* Configure channel 2 and 3 to trigger interrupts on capture compare values */
    ENCODER_TIMER->DIER = 0x00; // Clear all interrupts
    // Enable capture compare on CH2, CH3 and UIE
    ENCODER_TIMER->DIER |= ((0x01 << 0) | (0x01 << 2) | (0x01 << 3));

    ENCODER_TIMER->CCMR2 &= ~(0x03 << 0); // Reset capture compare 3 to output
    ENCODER_TIMER->CCMR2 &= ~((0x01 << 16) | (0x07 << 4)); // Reset output compare mode 3 to frozen
    
    ENCODER_TIMER->CCMR1 &= ~(0x03 << 8); // Reset capture compare 2 to output
    ENCODER_TIMER->CCMR1 &= ~((0x01 << 24) | (0x07 << 12)); // Reset output compare mode 2 to frozen

    // Set counter values for the interrupts to be triggered on for each channel 
    ENCODER_TIMER->CCR2 = 0;
    ENCODER_TIMER->CCR3 = maximumCount;
    
    // Enable counter
    ENCODER_TIMER->CR1 |= TIM_CR1_CEN;

    // Enable the interrupts
    HAL_NVIC_SetPriority(TIM1_CC_IRQn, 9, 0);
	HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);

    debug_prints("Initialised\r\n");
}

void encoder_enter_manual_override(void) {

    // Disable counter interrupts to prevent motor from stopping during manual override
    ENCODER_TIMER->DIER &= ~(TIM_DIER_CC2IE | TIM_DIER_CC3IE);
}

void encoder_exit_manual_override(void) {

    // Enable counter to let encoder update
    // Clear any interrupts that were triggered during the manual overried
    ENCODER_TIMER->SR = ~(TIM_SR_CC2IF | TIM_SR_CC3IF);
    ENCODER_TIMER->DIER |= (TIM_DIER_CC2IE | TIM_DIER_CC3IE);
}

void encoder_set_direction_positive(void) {
    // Set counter direction to up counting
    TIM1->CR1 &= ~(0x01 << 4);
}

void encoder_set_direction_negative(void) {
    // Set counter direction to down counting
    TIM1->CR1 |= (0x01 << 4);
}

uint32_t encoder_get_count(void) {
    return TIM1->CNT;
}

uint8_t encoder_at_minimum_distance(void) {
    return ENCODER_TIMER->CNT == 0;
}

uint8_t encoder_at_maximum_distance(void) {
    return ENCODER_TIMER->CNT == maximumCount;
}

void encoder_isr_reset_min_value(void) {
    // CCR2 is always mapped to 0. Thus only need to set counter to 0 to reset
    // minimum value
    ENCODER_TIMER->CNT = 0;
    encoder_set_direction_positive();
    debug_prints("Min Pos reset. CNT = 0\r\n");
}

void encoder_isr_reset_max_value(void) {
    maximumCount = ENCODER_TIMER->CNT;
    ENCODER_TIMER->CCR3 = maximumCount;
    encoder_set_direction_negative();
    char m[40];
    sprintf(m, "Max pos set to %i. Current CNT: %lu\r\n", maximumCount, ENCODER_TIMER->CNT);
    debug_prints(m);
}

void encoder_print_state(void) {

    char msg[70];
    // sprintf(msg, "isrCount: %i\r\n", isrCount);
    sprintf(msg, "Distance: %lu\t ISRCount: %i\r\n", encoder_distance_travelled(), isrCount);
    debug_prints(msg);
}