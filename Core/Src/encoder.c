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

/* STM32 Includes */

/* Private #defines */
#define ENCODER_PIN_RAW 4
#define ENCODER_PIN (ENCODER_PIN_RAW * 2)
#define ENCODER_PORT GPIOB
#define ENCODER_PORT_CLK_POS (0x01 << RCC_AHB2ENR_GPIOBEN)
#define ENCODER_IRQn EXTI4_IRQn

#define ENCODER_OFB_ERROR_1 0
#define ENCODER_OFB_ERROR_2 1
#define ENCODER_NEGATIVE (-1)
#define ENCODER_POSITIVE 1

#define ENCODER_MICROMETER 1000000
#define ENCODER_MILLIMETER 1000
#define ENCODER_CENTIMETER 100
#define ENCODER_METER 1
#define DISTANCE_UNIT ENCODER_MICROMETER

#define ENCODER_CIRCUMFERENCE 75398 // micrometers
#define ENCODER_NUM_GEAR_TEETH 16
#define ENCODER_DISTANCE_PER_TOOTH ((uint32_t) (ENCODER_CIRCUMFERENCE / ENCODER_NUM_GEAR_TEETH))

#define ENCODER_MINIMUM_DISTANCE 0
#define ENCODER_MAXIMUM_DISTANCE (ENCODER_DISTANCE_PER_TOOTH * ENCODER_NUM_GEAR_TEETH * 1)

/* Variable Declarations */
uint32_t distanceTravelled = 0;
uint32_t teethPassed       = 0;
uint8_t encoderStatus      = 0;
int encoderDirection   = ENCODER_POSITIVE;
uint16_t isrCount = 0;

/* Function prototypes */

void encoder_init(void) {

    // Initialise GPIO pin
    ENCODER_PORT->MODER  &= ~(0x11 << ENCODER_PIN); // Set pin to input mode
    ENCODER_PORT->OSPEEDR &= ~(0x11 << ENCODER_PIN); // Set pin to low speed
    ENCODER_PORT->PUPDR  &= ~(0x11 << ENCODER_PIN); // Set pin to no pull up/down
    ENCODER_PORT->OTYPER &= ~(0x01 << ENCODER_PIN_RAW); // Set pin to push-pull

    // Enable GPIO Clock
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Set up interrupt for GPIO line
    SYSCFG->EXTICR[1] &= ~(0x07 << 0); // Clear trigger line and set line for PA4
    SYSCFG->EXTICR[1] |= (0x01 << 0); // Clear trigger line and set line for PA4

    EXTI->RTSR1 |= EXTI_RTSR1_RT4; // Enable trigger on rising edge
    EXTI->FTSR1 &= ~EXTI_FTSR1_FT4; // Disable interrupt on falling edge
    EXTI->IMR1  |= EXTI_IMR1_IM4; // Enabe external interrupt for EXTI line

    // Configure interrupt priorities
    HAL_NVIC_SetPriority(ENCODER_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(ENCODER_IRQn);
}

void encoder_isr(void) {

    distanceTravelled = encoder_distance_travelled();
    char a[60];
    sprintf(a, "dt: %li\r\n", distanceTravelled);
    debug_prints(a);

    // Check for out of bounds error
    if ((distanceTravelled == ENCODER_MAXIMUM_DISTANCE) && (encoderDirection == ENCODER_POSITIVE)) {
        sprintf(a, "ret 1: ed: %i\r\n", encoderDirection);
        debug_prints(a);
        encoderStatus = ENCODER_AT_MAX_DISTANCE;
        return;
    }

    if ((distanceTravelled == ENCODER_MINIMUM_DISTANCE) && (encoderDirection == ENCODER_NEGATIVE)) {
        encoderStatus = ENCODER_AT_MIN_DISTANCE;
        sprintf(a, "ret 2: ed: %i\r\n", encoderDirection);
        debug_prints(a);
        return;
    }

    sprintf(a, "Encoder dir: %i\r\n", encoderDirection);
    debug_prints(a);
    teethPassed += encoderDirection; 
    isrCount++;
}

uint16_t encoder_get_isr_count(void) {
    return isrCount;
}

uint32_t encoder_distance_travelled(void) {
    return ((uint32_t) teethPassed) * ENCODER_DISTANCE_PER_TOOTH;
}

void encoder_set_direction_positive(void) {
    encoderDirection = ENCODER_POSITIVE;
}

void encoder_set_direction_negative(void) {
    encoderDirection = ENCODER_NEGATIVE;
}

uint8_t encoder_status(void) {
    return encoderStatus;
}

uint8_t encoder_at_minimum_distance(void) {
    return encoder_distance_travelled() == ENCODER_MINIMUM_DISTANCE;
}

uint8_t encoder_at_maximum_distance(void) {
    return encoder_distance_travelled() == ENCODER_MAXIMUM_DISTANCE;
}