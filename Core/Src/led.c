/**
 * @file led.c
 * @author Gian Barta-Dougall
 * @brief System file for led
 * @version 0.1
 * @date --
 * 
 * @copyright Copyright (c) 
 * 
 */
/* Public Includes */

/* Private Includes */
#include "led.h"
#include "hardware_config.h"

/* Private STM Includes */

/* Private #defines */
#define LED_ON 0x01
#define LED_OFF 0x02

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */
void led_hardware_init(void);
uint8_t led_get_state(GPIO_TypeDef* gpio, uint32_t pin);
void led_config(GPIO_TypeDef* gpio, uint32_t pin);

/* Public Functions */

void led_init(void) {

	// Initialise hardware 
	led_hardware_init();

}

/* Private Functions */

/**
 * @brief Initialise the hardware for the library.
 */
void led_hardware_init(void) {

	// Initialise ORANGE and RED LEDs
	led_config(LED_ORANGE_PORT, LED_ORANGE_PIN);
	led_config(LED_RED_PORT, LED_RED_PIN);
}

void led_config(GPIO_TypeDef* gpio, uint32_t pin) {

    gpio->MODER   &= ~(0x03 << (0x02 * pin)); // Reset pin mode
	gpio->MODER   |= (0x01 << (0x02 * pin)); // Set pin mode to output

	gpio->OTYPER &= (0x03 << (0x02 * pin)); // Set output to push-pull
	gpio->OSPEEDR &= (0x03 << (0x02 * pin)); // Set pin to low speed

    gpio->PUPDR   &= ~(0x03 << (0x02 * pin)); // Reset pull up pull down pin
    gpio->PUPDR   |= (0x02 << (0x02 * pin)); // Set pin to pull down
}

void led_on(uint8_t ledId) {

	// Set LED high
	switch (ledId) {
		case TEMPEST_LED_ORANGE:
			LED_ORANGE_PORT->BSRR |= (0x01 << LED_ORANGE_PIN);
			break;
		case TEMPEST_LED_RED:
			LED_RED_PORT->BSRR |= (0x01 << LED_RED_PIN);
			break;
		default:
			// No action. The LED ID was invalid. 
			break;
	}

}

void led_off(uint8_t ledId) {

	// Set the LED low
	switch (ledId) {
		case TEMPEST_LED_ORANGE:
			// Add 16 as the reset part of this register are from bits 16-31
			LED_ORANGE_PORT->BSRR |= (0x01 << (LED_ORANGE_PIN + 16));
			break;
		case TEMPEST_LED_RED:
			// Add 16 as the reset part of this register are from bits 16-31
			LED_RED_PORT->BSRR |= (0x01 << (LED_RED_PIN + 16));
			break;
		default:
			// No action. The LED ID was invalid. 
			break;
	}
}

void led_toggle(uint8_t ledId) {

	// Toggle state of the LED
	switch (ledId) {
		case TEMPEST_LED_ORANGE:

			// Check the current state of the LED
			if (led_get_state(LED_ORANGE_PORT, LED_ORANGE_PIN) == LED_ON) {
				led_off(ledId);
			} else {
				led_on(ledId);
			}

			break;
		case TEMPEST_LED_RED:

			// Check the current state of the LED
			if (led_get_state(LED_RED_PORT, LED_RED_PIN) == LED_ON) {
				led_off(ledId);
			} else {
				led_on(ledId);
			}

			break;
		default:
			// No action. The LED ID was invalid. 
			break;
	}
}

uint8_t led_get_state(GPIO_TypeDef* gpio, uint32_t pin) {
	
	// Check the current state of the LED
	if ((gpio->ODR & (0x01 << pin)) != 0x00) {
		return LED_ON;
	}

	return LED_OFF;
}