/**
 * @file ambient_light_sensor.c
 * @author Gian Barta-Dougall
 * @brief Peripheral driver for a generic ambient light sensor using an
 * integrator circuit
 * @version 0.1
 * @date 2022-06-25
 *
 * @copyright Copyright (c) 2022
 *
 */

/* Public Includes */

/* Private Includes */
#include "ambient_light_sensor_config.h"
#include "debug_log.h"
#include "interrupts_config.h"

/* STM32 Includes */

/* Private Marcos */
#define ID_INVALID(id) ((id < AL_SENSOR_OFFSET) || (id > (NUM_AL_SENSORS - 1 + AL_SENSOR_OFFSET)))

// If light is found when the count equals the cut off frequency, the count
// is set to the maximum count. If the light is not found when the count equals
// the cutoff frequency the count is set to 0.
#define CUT_OFF_COUNT 3
#define MAX_COUNT     (CUT_OFF_COUNT * 2)

/* Variable Declarations */

// Keeps track of how many times in a row ambient light has or has not been found.
// This ensures that a light from a passing car or a dark shadow won't fool the
// sensor into recording a false state
uint8_t count[NUM_AL_SENSORS] = {0};

/* Function prototypes */

void als_init(void) {

    // This function needs to work out whether the als sensor has been connected or not
}

/**
 * @brief Setting the pin to input allows the capacitor in the circuit to discharge
 * creating a voltage that can be read to determine if there was ambient light
 */
void als_mode_input(uint8_t alsId) {

    // Ensure the sensor ID is valid
    if (ID_INVALID(alsId)) {
        return;
    }

    uint8_t index = alsId - AL_SENSOR_OFFSET;
    alSensors[index].port->MODER &= ~(0x03 << alSensors[index].pin);
}

/**
 * @brief Setting the pin to analogue mode prevents the capacitor from dishcharging
 * too quickly allowing it to slowly charge based on the amount of ambient ligh
 */
void als_mode_reset(uint8_t alsId) {

    // Ensure the sensor ID is valid
    if (ID_INVALID(alsId)) {
        return;
    }

    uint8_t index = alsId - AL_SENSOR_OFFSET;
    alSensors[index].port->MODER |= (0x03 << alSensors[index].pin);
}

/**
 * @brief Update the count that keeps track of how many times in a row
 * ambient light has either been found or not been found
 */
void als_update_status(uint8_t alsId) {

    // Ensure the sensor ID is valid
    if (ID_INVALID(alsId)) {
        return;
    }

    // Calculate the index of the sensor from the ID
    uint8_t index = alsId - AL_SENSOR_OFFSET;

    // Read sensor
    uint8_t lightFound = (alSensors[index].port->IDR & (0x01 << alSensors[index].pin * 2)) != 0;

    // Update count
    if (lightFound) {
        count[index] = (count[index] == CUT_OFF_COUNT) ? MAX_COUNT : (count[index] + 1);
    } else {
        count[index] = (count[index] == CUT_OFF_COUNT) ? 0 : (count[index] - 1);
    }

    // Discharge capacitor to 0V
    alSensors[index].port->MODER &= ~(0x03 << alSensors[index].pin);      // Reset pin mode
    alSensors[index].port->MODER |= (0x01 << alSensors[index].pin);       // Set pin mode to output
    alSensors[index].port->BSRR |= (0x01 << (alSensors[index].pin + 16)); // Set output low to discharge capacitor
}

/**
 * @brief Returns state of ambient light sensor
 *
 * @return uint8_t Returns 1 if ambient light sensor detects light else 0
 */
uint8_t als_light_found(uint8_t alsId) {

    // Ensure the sensor ID is valid
    if (ID_INVALID(alsId)) {
        return 255;
    }

    // Return whether light has been detected or not
    uint8_t index = alsId - AL_SENSOR_OFFSET;
    return count[index] >= CUT_OFF_COUNT;
}
