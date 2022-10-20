#ifndef ENCODER_CONFIG_H
#define ENCODER_CONFIG_H

#include "encoder.h"
#include "hardware_config.h"

/* Includes for repeated recipes */
#define UNSET 0

// This number is arbitrary however setting it a number > 0 prevents
// wrapping around to really large numbers if the encoder for some
// reason goes slightly below the minimum count
#define ZERO_COUNT 1000

typedef struct Encoder {
    const uint8_t id;
    GPIO_TypeDef* port;
    const uint32_t pin;
    TIM_TypeDef* timer;
    uint32_t minCount;
    uint32_t maxCount;
} Encoder;

/* Initialise encoder configurations */

#if (VERSION_MAJOR == 0)

const Encoder encoder1 = {
    .id       = ENCODER_1_ID,
    .port     = HC_ENCODER_1_PORT,
    .pin      = HC_ENCODER_1_PIN,
    .timer    = HC_ENCODER_1_TIMER,
    .minCount = ZERO_COUNT,
    .maxCount = ZERO_COUNT + 10,
};

const Encoder encoder2 = {
    .id    = ENCODER_2_ID,
    .port  = HC_ENCODER_2_PORT,
    .pin   = HC_ENCODER_2_PIN,
    .timer = HC_ENCODER_2_TIMER,
};

    #define NUM_ENCODERS 2
#endif

Encoder encoders[NUM_ENCODERS] = {encoder1, encoder2};

#ifndef NUM_ENCODERS
    #error Number of encoders has not been defined
#endif

#endif // ENCODER_CONFIG_H