/**
 * @file ambient_light_sensor.c
 * @author Gian Barta-Dougall
 * @brief Peripheral driver for a generic ambient light sensor using an integrator circuit
 * @version 0.1
 * @date 2022-06-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/* Public Includes */

/* Private Includes */
#include "ambient_light_sensor.h"
#include "debug_log.h"
#include "interrupts_config.h"

/* STM32 Includes */

/* Private #defines */
#define ALS_PIN_RAW 12
#define ALS_PIN (ALS_PIN_RAW * 2)
#define ALS_PORT GPIOA
#define ALS_PORT_CLK_POS (0x01 << RCC_AHB2ENR_GPIOBEN)
#define ALS_IRQn EXTI5_IRQn

// The minimum number of times the sensor must read either low or high for before
// the sensor will state whether ambient light exists or not. Until this number is
// reached for either low or high, the ambient light sensor will state the light
// level is undetermined
#define ALS_MINIMUM_STREAK_COUNT 5

/* Variable Declarations */
uint8_t ambientLightDetectedStreak = 0;
uint8_t ambientLightNotDetectedStreak = 0;

/* Function prototypes */
void ambient_light_sensor_hardware_init(void);

void ambient_light_sensor_init(void) {

    // Initialise hardware
    ambient_light_sensor_hardware_init();
}

void ambient_light_sensor_hardware_init(void) {
    
    // Initialise GPIO pin
    ALS_PORT->MODER   &= ~(0x03 << ALS_PIN); // Set pin to analogue mode (High Z)
    ALS_PORT->MODER   |= (0x01 << ALS_PIN);
    ALS_PORT->OSPEEDR &= ~(0x03 << ALS_PIN); // Set pin to low speed
    ALS_PORT->PUPDR   &= ~(0x03 << ALS_PIN); // Set pin to reset pupdr
    ALS_PORT->OTYPER  &= ~(0x01 << ALS_PIN_RAW); // Set pin to push pull

    // Enable clock
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

void ambient_light_sensor_isr_s1(void) {
    
    // Set data pin to input mode. This will allow the IDR to be read to determine
    // whether the data pin was high or low
    ALS_PORT->MODER &= ~(0x03 << ALS_PIN); 
}

enum AmbientLightLevel ambient_light_read(void) {
    if (ambientLightDetectedStreak == ALS_MINIMUM_STREAK_COUNT) {
        return HIGH;
    } else if (ambientLightNotDetectedStreak == ALS_MINIMUM_STREAK_COUNT) {
        return LOW;
    }

    return UNDETERMINED;
}

void ambient_light_sensor_isr_s2(void) {

    if ((ALS_PORT->IDR & (0x01 << ALS_PIN_RAW)) == (0x01 << ALS_PIN_RAW)) {
        ambientLightNotDetectedStreak = 0;
        debug_prints("ambient light detected\r\n");
        
        if (ambientLightDetectedStreak < ALS_MINIMUM_STREAK_COUNT) {
            ambientLightDetectedStreak++;;
        }
    } else  {
        ambientLightDetectedStreak = 0;
        debug_prints("ambient light not detected\r\n");
        // Increment streak if current count is less than required count
        if (ambientLightNotDetectedStreak < ALS_MINIMUM_STREAK_COUNT) {
            ambientLightNotDetectedStreak++;
        }
    }

    // Set the data pin to an output and set output low to discharge the data line
    ALS_PORT->MODER |= (0x01 << ALS_PIN); // Set pin to output mode
    ALS_PORT->ODR &= ~(0x01 << ALS_PIN_RAW); // Set pin low to discharge capacitor
}

void ambient_light_sensor_isr_s3(void) {

    // Set pin to high impendance mode. This will allow the sensor to store charge
    // on the data pin so it can be read at a later time
    ALS_PORT->MODER |= (0x03 << ALS_PIN); // Set pin to analogue mode (High Z)
}


