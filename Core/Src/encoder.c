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
#define ENCODER_PIN_RAW 4
#define ENCODER_PIN (ENCODER_PIN_RAW * 2)
#define ENCODER_PORT GPIOB
#define ENCODER_PORT_CLK_POS (0x01 << RCC_AHB2ENR_GPIOBEN)
#define ENCODER_IRQn EXTI4_IRQn

#define ENCODER_OFB_ERROR_1 0
#define ENCODER_OFB_ERROR_2 1

#define ENCODER_MICROMETER 1000000
#define ENCODER_MILLIMETER 1000
#define ENCODER_CENTIMETER 100
#define ENCODER_METER 1
#define DISTANCE_UNIT ENCODER_MICROMETER

#define ENCODER_CIRCUMFERENCE 75398 // micrometers
#define ENCODER_NUM_GEAR_TEETH 8
#define ENCODER_DISTANCE_PER_TOOTH ((uint32_t) (ENCODER_CIRCUMFERENCE / ENCODER_NUM_GEAR_TEETH))

#define ENCODER_TIMER TIM1
#define ENCODER_MAX_COUNT 88

// The maximum distance the object needs to travel in micrometers
#define ENOCDER_OBJECT_DISTANCE 1450000 // (1.45m) // 867054 - 46 ISR
#define ENCODER_MINIMUM_DISTANCE 0
#define ENCODER_MAXIMUM_DISTANCE (((uint32_t) (ENOCDER_OBJECT_DISTANCE / ENCODER_CIRCUMFERENCE)) * ENCODER_DISTANCE_PER_TOOTH * ENCODER_NUM_GEAR_TEETH)

#define ENCODER_UNLOCKED 0
#define ENCODER_LOCKED 1

/* Variable Declarations */
uint32_t distanceTravelled = 0;
uint32_t teethPassed       = 0;
uint8_t encoderStatus      = 0;
uint16_t isrCount    = 0;
uint8_t encoderLock  = ENCODER_LOCKED;
uint16_t absISRCount = 0;
uint32_t time1 = 0;
uint32_t time2 = 0;

/* Function prototypes */
void encoder_timer_init(void);

void encoder_init(void) {

    // Initialise internal comparator
    comparator_init();

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

    // Reroute comparator output to TIM1 input
    ENCODER_TIMER->OR1 |= (0x01 << 4);

    // Set TIM1 to input and map channel 1 to trigger input 1 (TI1)
    ENCODER_TIMER->CCMR1 &= ~(0x03 << 0); // Reset capture compare
    ENCODER_TIMER->CCMR1 |= (0x01 << 0); // Set capture compare to input (IC1 mapped to TI1)
    
    // Configure slave mode control
    ENCODER_TIMER->SMCR &= ~(0x07 << 4); // Reset trigger selection
    ENCODER_TIMER->SMCR |= (0x05 << 4); // Set trigger to Filtered Timer Input 1 (TI1FP1)
    ENCODER_TIMER->SMCR &= ~((0x01 << 16) | 0x07); // Reset slave mode selection
    ENCODER_TIMER->SMCR |= 0x07; // Set rising edge of selected trigger to clock the counter

    ENCODER_TIMER->CNT = 0; // Reset counter to 0
    ENCODER_TIMER->CR1 |= 0x01; // Enable the counter
    
    /* Configure channel 2 and 3 to trigger interrupts on capture compare values */
    ENCODER_TIMER->DIER |= (0x01 << 3); // Enable Capture compare 3 interrupt
    ENCODER_TIMER->DIER |= (0x01 << 2); // Enable Capture compare 2 interrupt
    ENCODER_TIMER->DIER |= (0x01 << 0); // Enable interrupt updates

    ENCODER_TIMER->CCMR2 &= ~(0x03 << 0); // Reset capture compare 3 to output
    ENCODER_TIMER->CCMR2 &= ~((0x01 << 16) | (0x07 << 4)); // Reset output compare mode 3 to frozen
    
    ENCODER_TIMER->CCMR1 &= ~(0x03 << 8); // Reset capture compare 2 to output
    ENCODER_TIMER->CCMR1 &= ~((0x01 << 24) | (0x07 << 12)); // Reset output compare mode 2 to frozen

    // Set counter values for the interrupts to be triggered on for each channel 
    ENCODER_TIMER->CCR3 = 10;
    ENCODER_TIMER->CCR2 = 0;

    // Enable the interrupts
    HAL_NVIC_SetPriority(TIM1_CC_IRQn, 9, 0);
	HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
}

uint32_t encoder_distance_travelled(void) {
    
    uint32_t distance = 0;
    uint8_t teethPassedCopy = teethPassed;
    int revolutions = teethPassedCopy / ENCODER_NUM_GEAR_TEETH;
    int partialRevolutions = teethPassedCopy % ENCODER_NUM_GEAR_TEETH; 
    // Add an extra loop to account for partial revolutions
    if (partialRevolutions != 0) {
        revolutions++;
    }

    for (int i = revolutions; i > 0; i--) {
        
        if (i == 1) {
            distance += ((43 - i) * partialRevolutions) / ENCODER_NUM_GEAR_TEETH;
        } else {
            distance += (43 - i);
        }
    }

    return (distance * 314) * 10;

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

uint8_t encoder_status(void) {
    return encoderStatus;
}

uint8_t encoder_at_minimum_distance(void) {
    return ENCODER_TIMER->CNT == 0;
}

uint8_t encoder_at_maximum_distance(void) {
    return ENCODER_TIMER->CNT == ENCODER_MAX_COUNT;
}

void encoder_reset(void) {
    teethPassed = 0;
    isrCount = 0;
}

void encoder_print_state(void) {

    char msg[70];
    // sprintf(msg, "isrCount: %i\r\n", isrCount);
    sprintf(msg, "Distance: %lu\t ISRCount: %i\r\n", encoder_distance_travelled(), isrCount);
    debug_prints(msg);
}

void encoder_lock(void) {
    encoderLock = ENCODER_LOCKED;
}

void encoder_unlock(void) {
    encoderLock = ENCODER_UNLOCKED;
}