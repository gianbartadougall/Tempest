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
#include "encoder_config.h"
#include "utilities.h"

/* STM32 Includes */

#define ENCODER_ID_INVALID(id)  ((id < ENCODER_ID_OFFSET) || (id > (NUM_ENCODERS - 1 + ENCODER_ID_OFFSET)))
#define ENCODER_ID_TO_INDEX(id) (id - ENCODER_ID_OFFSET)

#define ASSERT_VALID_ENCODER_ID(id)                                                      \
    do {                                                                                 \
        if ((id < ENCODER_ID_OFFSET) || (id > (NUM_ENCODERS - 1 + ENCODER_ID_OFFSET))) { \
            char msg[100];                                                               \
            sprintf(msg, "Invalid ID: File %s line number %d\r\n", __FILE__, __LINE__);  \
            debug_prints(msg);                                                           \
            return;                                                                      \
        }                                                                                \
    } while (0)

/* Variable Declarations */
uint32_t encoderTaskFlags = 0;

/* Function prototypes */
void encoder_timer_init(void);

void encoder_init(void) {

    // Reset all the timer counts
    for (uint8_t i = 0; i < NUM_ENCODERS; i++) {
        encoder_enable(encoders[i].id);
    }
}

uint8_t encoder_probe_connection(uint8_t encoderId) {

    if (ENCODER_ID_INVALID(encoderId)) {
        return DISCONNECTED;
    }

    // The encoder connects to the micrcontroller through a comparator with a series resistor
    // To probe whether the encoder is connected to the microcontroller, a high or low voltage
    // will be output and then that voltage will be read back by changing the pin to an input.
    // If the encoder is not connected, the input read will be the same as the output written.
    // If the encoder is connected, the input read will not be the same as the output written
    // for either high or low
    uint8_t index  = ENCODER_ID_TO_INDEX(encoderId);
    uint8_t status = DISCONNECTED;

    for (uint8_t i = 0; i < 1; i++) {

        // Description: Check if writing a high also returns a high when reading

        // Set pin mode to output and set pin high
        SET_PIN_MODE_INPUT(encoders[index].port, encoders[index].pin);
        SET_PIN_MODE_OUTPUT(encoders[index].port, encoders[index].pin);
        SET_PIN_TYPE_PUSH_PULL(encoders[index].port, encoders[index].pin);

        // Check high voltage first loop and low voltage on second loop
        if (i == 0) {
            SET_PIN_HIGH(encoders[index].port, encoders[index].pin);
        } else {
            SET_PIN_LOW(encoders[index].port, encoders[index].pin);
        }

        HAL_Delay(5); // Delay for 5ms so capacitor can fully charge

        // Set pin to high impedence
        SET_PIN_MODE_ANALOGUE(encoders[index].port, encoders[index].pin);

        // Delay for 5ms so if encoder is connected, it has time to fully discharge the charge
        // that was just output on the pin
        HAL_Delay(5);

        // Need to set the pin low to discharge any charge that has accumated on the wire
        // on the outside of the GPIO pin. If you don't do this and the encoder is not
        // connected, it will somtimes read high even when there is no capacitor connected.
        // Assuming it's because charge has accumulated on the actual pin. Setting the output
        // low for a short period fixes this
        SET_PIN_LOW(encoders[index].port, encoders[index].pin);
        HAL_Delay(1);

        // Set the mode to input and read if the charge on the pin
        SET_PIN_MODE_INPUT(encoders[index].port, encoders[index].pin);
        HAL_Delay(1); // Delay for 1ms so IDR can update

        // Check for pin being low on first loop and pin being high on second loop
        if (i == 0 && PIN_IDR_IS_LOW(encoders[index].port, encoders[index].pin)) {
            status = CONNECTED;
        }

        // if (i == 1 && PIN_IS_HIGH(encoders[index].port, encoders[index].pin)) {
        //     status = CONNECTED;
        // }
    }

    // Revert pins back to original function
    // SET_PIN_MODE_INPUT(encoders[index].port, encoders[index].pin);
    // SET_PIN_MODE_ALTERNATE_FUNCTION(encoders[index].port, encoders[index].pin);

    return status;
}

void encoder_enable(uint8_t encoderId) {

    ASSERT_VALID_ENCODER_ID(encoderId);

    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);

    // Reset all the registers in the timer and set the count to 0
    encoders[index].timer->EGR |= (TIM_EGR_UG);

    // Set the interrupt values to default values. Reset timer count
    // to zero point
    HC_ENCODER_1_TIMER->CCR2   = ZERO_COUNT;
    HC_ENCODER_1_TIMER->CCR3   = ZERO_COUNT + 10;
    encoders[index].timer->CNT = ZERO_COUNT;

    // Clear pending interrupts
    encoders[index].timer->SR = ~TIM_SR_CC2IF;
    encoders[index].timer->SR = ~TIM_SR_CC3IF;

    // Enable interrupts
    encoders[index].timer->DIER |= (TIM_DIER_CC2IE | TIM_DIER_CC3IE);

    // Start the timer
    encoders[index].timer->CR1 |= (TIM_CR1_CEN);
}

void encoder_disable(uint8_t encoderId) {

    ASSERT_VALID_ENCODER_ID(encoderId);

    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);
    encoders[index].timer->CR1 &= ~(TIM_CR1_CEN);
    encoders[index].timer->DIER &= ~(TIM_DIER_CC2IE | TIM_DIER_CC3IE);
}

void encoder_set_direction_up(uint8_t encoderId) {

    ASSERT_VALID_ENCODER_ID(encoderId);

    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);
    // The maximum height of the blind is the 0 point. So to go upwards, the timer counter
    // needs to count down
    SET_TIMER_DIRECTION_COUNT_DOWN(encoders[index].timer);
}

void encoder_set_direction_down(uint8_t encoderId) {

    ASSERT_VALID_ENCODER_ID(encoderId);

    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);

    // The maximum height of the blind is the 0 point. So to go downwards, the timer counter
    // needs to count up
    SET_TIMER_DIRECTION_COUNT_UP(encoders[index].timer);
}

uint32_t encoder_get_count(uint8_t encoderId) {

    if (ENCODER_ID_INVALID(encoderId)) {
        return UINT_32_BIT_MAX_VALUE;
    }

    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);
    return encoders[index].timer->CNT;
}

uint8_t encoder_at_max_height(uint8_t encoderId) {

    if (ENCODER_ID_INVALID(encoderId)) {
        return INVALID_ID;
    }

    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);
    char m[60];
    sprintf(m, "CNT: %li\tMax Height: %li\r\n", encoders[index].timer->CNT, encoders[index].minCount);
    debug_prints(m);

    return encoders[index].timer->CNT == encoders[index].minCount ? TRUE : FALSE;
}

uint8_t encoder_at_min_height(uint8_t encoderId) {

    if (ENCODER_ID_INVALID(encoderId)) {
        return INVALID_ID;
    }

    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);

    char m[60];
    sprintf(m, "CNT: %li\tMin Height: %li\r\n", encoders[index].timer->CNT, encoders[index].maxCount);
    debug_prints(m);

    return (encoders[index].timer->CNT == encoders[index].maxCount) ? TRUE : FALSE;
}

void encoder_set_max_height(uint8_t encoderId) {

    if (ENCODER_ID_INVALID(encoderId)) {
        return;
    }

    // CCR2 is always mapped to 0. Thus only need to set counter to 0 to reset
    // minimum value
    uint8_t index              = ENCODER_ID_TO_INDEX(encoderId);
    encoders[index].minCount   = ZERO_COUNT;
    encoders[index].timer->CNT = ZERO_COUNT;
    char m[60];
    sprintf(m, "max height CNT: %li\r\n", encoders[index].timer->CNT);
    debug_prints(m);
}

void encoder_set_min_height(uint8_t encoderId) {

    if (ENCODER_ID_INVALID(encoderId)) {
        return;
    }

    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);

    encoders[index].maxCount    = encoders[index].timer->CNT;
    encoders[index].timer->CCR3 = encoders[index].maxCount;
}

void encoder_disable_limits(uint8_t encoderId) {

    if (ENCODER_ID_INVALID(encoderId)) {
        return;
    }

    uint8_t index            = ENCODER_ID_TO_INDEX(encoderId);
    encoders[index].minCount = UNSET;
    encoders[index].maxCount = UNSET;
}

uint8_t encoder_limits_are_valid(uint8_t encoderId) {

    if (ENCODER_ID_INVALID(encoderId)) {
        return INVALID_ID;
    }

    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);

    if (encoders[index].minCount == UNSET || encoders[index].maxCount == UNSET) {
        return FALSE;
    }

    return TRUE;
}

void encoder_limit_reached_isr(uint8_t encoderId) {

    if (ENCODER_ID_INVALID(encoderId)) {
        return;
    }

    debug_prints("Limit reached\r\n");
    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);
    if ((encoders[index].minCount == UNSET) || (encoders[index].maxCount == UNSET)) {
        return;
    }

    switch (index) {
        case 0:
            FLAG_SET(encoderTaskFlags, ENCODER_1_LIMIT_REACHED);
            break;
        case 1:
            FLAG_SET(encoderTaskFlags, ENCODER_2_LIMIT_REACHED);
            break;

        default:
            break;
    }
}

uint32_t encoder_get_lower_bound_interrupt(uint8_t encoderId) {

    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);

    if (index == INVALID_ID) {
        return __32_BIT_MAX_COUNT;
    }

    return encoders[index].timer->CCR2;
}

uint32_t encoder_get_upper_bound_interrupt(uint8_t encoderId) {

    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);

    if (index == INVALID_ID) {
        return __32_BIT_MAX_COUNT;
    }

    return encoders[index].timer->CCR3;
}

void encoder_set_lower_bound_interrupt(uint8_t encoderId) {

    ASSERT_VALID_ENCODER_ID(encoderId);

    uint8_t index               = ENCODER_ID_TO_INDEX(encoderId);
    encoders[index].timer->CCR2 = encoders[index].timer->CNT;
}

void encoder_set_upper_bound_interrupt(uint8_t encoderId) {

    ASSERT_VALID_ENCODER_ID(encoderId);

    uint8_t index               = ENCODER_ID_TO_INDEX(encoderId);
    encoders[index].timer->CCR3 = ZERO_COUNT;
    encoders[index].timer->CNT  = ZERO_COUNT;
}

void encoder_enable_interrupts(uint8_t encoderId) {

    ASSERT_VALID_ENCODER_ID(encoderId);

    // Clear any pending interrupts and then enable the interrupts
    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);
    encoders[index].timer->SR &= ~(TIM_DIER_CC2IE | TIM_DIER_CC3IE);
    encoders[index].timer->DIER |= (TIM_DIER_CC2IE | TIM_DIER_CC3IE);
}

void encoder_disable_interrupts(uint8_t encoderId) {

    ASSERT_VALID_ENCODER_ID(encoderId);

    // Clear any pending interrupts and disable interrupts
    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);
    encoders[index].timer->SR &= ~(TIM_DIER_CC2IE | TIM_DIER_CC3IE);
    encoders[index].timer->DIER = 0;
}

uint8_t encoder_get_state(uint8_t encoderId) {

    if (ENCODER_ID_INVALID(encoderId)) {
        return INVALID_ID;
    }

    uint8_t index = ENCODER_ID_TO_INDEX(encoderId);

    // Disable the timer to ensure the following does not affect the count
    encoders[index].timer->CR1 &= ~TIM_CR1_CEN;

    // Encoders are in alternate function mode. Set the pin mode to input
    // to check the state of the pin. Apply a weak pull down resistor to
    // ensure pin is low if there is no encoder connected
    SET_PIN_MODE_INPUT(encoders[index].port, encoders[index].pin);
    SET_PIN_PULL_AS_PULL_DOWN(encoders[index].port, encoders[index].pin);

    // Wait short duration for IDR to update
    HAL_Delay(1);

    // Record the IDR
    uint8_t encoderInput = PIN_IDR_STATE(encoders[index].port, encoders[index].pin);

    // Set pin back to original state
    SET_PIN_PULL_AS_NONE(encoders[index].port, encoders[index].pin);
    SET_PIN_MODE_ALTERNATE_FUNCTION(encoders[index].port, encoders[index].pin);

    // Reenable the timer
    encoders[index].timer->CR1 |= TIM_CR1_CEN;

    return encoderInput;
}