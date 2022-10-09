#ifndef LED_CONFIG_H
#define LED_CONFIG_H

#include "hardware_config.h"
#include "led.h"

typedef struct LED {
    const uint8_t id;
    GPIO_TypeDef* port;
    uint32_t pin;
} LED;

/* Initialise LEDs */

#define NUM_LEDS 3

const LED redLED = {
    .id   = LED_RED_ID,
    .port = HC_LED_RED_PORT,
    .pin  = HC_LED_RED_PIN,
};

const LED greenLED = {
    .id   = LED_GREEN_ID,
    .port = HC_LED_GREEN_PORT,
    .pin  = HC_LED_GREEN_PIN,
};

const LED orangeLED = {
    .id   = LED_ORANGE_ID,
    .port = HC_LED_ORANGE_PORT,
    .pin  = HC_LED_ORANGE_PIN,
};

LED leds[NUM_LEDS] = {redLED, greenLED, orangeLED};

#endif // LED_CONFIG_H