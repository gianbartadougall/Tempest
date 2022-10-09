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
#include "adc_config.h"

/* STM32 Includes */

/* Private Marcos */
#define ID_INVALID(id) ((id < AL_SENSOR_OFFSET) || (id > (NUM_AL_SENSORS - 1 + AL_SENSOR_OFFSET)))

// If light is found when the count equals the cut off frequency, the count
// is set to the maximum count. If the light is not found when the count equals
// the cutoff frequency the count is set to 0.
#define CUT_OFF_COUNT 3
#define MAX_COUNT     (CUT_OFF_COUNT * 2)

/* Variable Declarations */
extern uint32_t ambientLightSensorFlag;

// Keeps track of how many times in a row ambient light has or has not been found.
// This ensures that a light from a passing car or a dark shadow won't fool the
// sensor into recording a false state
uint8_t count[NUM_AL_SENSORS] = {0};

// List keeps track of the state of each sensor
enum ALSensorState { DISCONNECTED, CONNECTED };
uint8_t sensorStatus[NUM_AL_SENSORS] = {DISCONNECTED};

/* Function prototypes */

/**
 * @brief Setting the pin to input allows the capacitor in the circuit to discharge
 * creating a voltage that can be read to determine if there was ambient light
 */
void al_sensor_set_mode_read(uint8_t alSensorId) {
    debug_prints("ALS: Set to input mode\r\n");

    // Ensure the sensor ID is valid
    if (ID_INVALID(alSensorId)) {
        return;
    }

    uint8_t index = alSensorId - AL_SENSOR_OFFSET;
    alSensors[index].port->MODER &= ~(0x03 << (alSensors[index].pin * 2));
}

/**
 * @brief Setting the pin to analogue mode prevents the capacitor from dishcharging
 * too quickly allowing it to slowly charge based on the amount of ambient light
 */
void al_sensor_start_new_cycle(uint8_t alSensorId) {
    debug_prints("ALS: Starting new cycle\r\n");
    // Ensure the sensor ID is valid
    if (ID_INVALID(alSensorId)) {
        return;
    }

    uint8_t index = alSensorId - AL_SENSOR_OFFSET;
    alSensors[index].port->MODER |= (0x03 << (alSensors[index].pin * 2));

    // Capacitor should be fully disharged. Confirm sensor is still connected by
    // reading voltage on the data pin
    for (uint8_t i = 0; i < 10; i++) {
        if (adc_config_adc1_convert() > 50) {
            // debug_prints("Could not detect sensor\r\n");
            sensorStatus[index] = DISCONNECTED;
            return;
        }
    }

    sensorStatus[index] = CONNECTED;
}

/**
 * @brief Update the count that keeps track of how many times in a row
 * ambient light has either been found or not been found
 */
void al_sensor_read(uint8_t alSensorId) {
    debug_prints("ALS: Reading\r\n");
    // Ensure the sensor ID is valid
    if (ID_INVALID(alSensorId)) {
        return;
    }

    // Calculate the index of the sensor from the ID
    uint8_t index = alSensorId - AL_SENSOR_OFFSET;

    // Read sensor
    uint8_t lightFound = (alSensors[index].port->IDR & (0x01 << (alSensors[index].pin * 2))) != 0;

    // Update count
    if (lightFound) {
        count[index] = (count[index] == CUT_OFF_COUNT) ? MAX_COUNT : (count[index] + 1);
    } else {
        count[index] = (count[index] == CUT_OFF_COUNT) ? 0 : (count[index] - 1);
    }
}

/**
 * @brief The sensor is reset by fully discharging the capacitor. This can be done by setting the
 * data pin low
 */
void al_sensor_reset(uint8_t alSensorId) {
    debug_prints("ALS: Resetting\r\n");
    // Ensure the sensor ID is valid
    if (ID_INVALID(alSensorId)) {
        return;
    }

    uint8_t index = alSensorId - AL_SENSOR_OFFSET;
    alSensors[index].port->MODER &= ~(0x03 << (alSensors[index].pin * 2)); // Reset pin mode
    alSensors[index].port->MODER |= (0x01 << (alSensors[index].pin * 2));  // Set pin mode to output
    alSensors[index].port->BSRR |= (0x01 << (alSensors[index].pin + 16));  // Set output low to discharge capacitor
}

/**
 * @brief Returns state of ambient light sensor
 *
 * @return uint8_t Returns 1 if ambient light sensor detects light else 0
 */
uint8_t al_sensor_status(uint8_t alSensorId) {

    // Ensure the sensor ID is valid
    if (ID_INVALID(alSensorId)) {
        return 255;
    }

    // Return whether light has been detected or not
    uint8_t index = alSensorId - AL_SENSOR_OFFSET;

    if (sensorStatus[index] == DISCONNECTED) {
        return 254;
    }

    return (count[index] >= CUT_OFF_COUNT) ? 1 : 0;
}

void al_sensor_process_flags(void) {

    if (ambientLightSensorFlag & (0x01 << 0)) {
        al_sensor_reset(AL_SENSOR_1);
        ambientLightSensorFlag &= ~(0x01 << 0);
    }

    if (ambientLightSensorFlag & (0x01 << 1)) {
        al_sensor_start_new_cycle(AL_SENSOR_1);
        ambientLightSensorFlag &= ~(0x01 << 1);
    }

    if (ambientLightSensorFlag & (0x01 << 2)) {
        al_sensor_set_mode_read(AL_SENSOR_1);
        ambientLightSensorFlag &= ~(0x01 << 2);
    }

    if (ambientLightSensorFlag & (0x01 << 3)) {
        al_sensor_read(AL_SENSOR_1);
        ambientLightSensorFlag &= ~(0x01 << 3);
    }
}