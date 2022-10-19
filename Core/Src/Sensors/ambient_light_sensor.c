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
#include "utilities.h"
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
uint8_t sensorStatus[NUM_AL_SENSORS] = {DISCONNECTED};

/* Function prototypes */

/**
 * @brief Setting the pin to input allows the capacitor in the circuit to discharge
 * creating a voltage that can be read to determine if there was ambient light
 */
void al_sensor_read_capacitor(uint8_t alSensorId) {
    // Ensure the sensor ID is valid
    if (ID_INVALID(alSensorId)) {
        return;
    }

    uint8_t si = alSensorId - AL_SENSOR_OFFSET;
    SET_PIN_MODE_INPUT(alSensors[si].port, alSensors[si].pin);

    HAL_Delay(1); // Wait 1ms for IDR to update

    // Update count
    if (PIN_IS_HIGH(alSensors[si].port, alSensors[si].pin)) {
        if (count[si] < MAX_COUNT) {
            count[si] = (count[si] == CUT_OFF_COUNT) ? MAX_COUNT : (count[si] + 1);
        }
    } else {
        if (count[si] > 0) {
            count[si] = (count[si] == CUT_OFF_COUNT) ? 0 : (count[si] - 1);
        }
    }
}

/**
 * @brief Setting the pin to analogue mode prevents the capacitor from dishcharging
 * too quickly allowing it to slowly charge based on the amount of ambient light
 */
void al_sensor_record_ambient_light(uint8_t alSensorId) {
    // Ensure the sensor ID is valid
    if (ID_INVALID(alSensorId)) {
        return;
    }

    uint8_t si = alSensorId - AL_SENSOR_OFFSET;
    SET_PIN_MODE_ANALOGUE(alSensors[si].port, alSensors[si].pin);
}

/**
 * @brief The sensor is reset by fully discharging the capacitor. This can be done by setting the
 * data pin low
 */
void al_sensor_discharge_capacitor(uint8_t alSensorId) {
    // Ensure the sensor ID is valid
    if (ID_INVALID(alSensorId)) {
        return;
    }

    uint8_t si = alSensorId - AL_SENSOR_OFFSET;
    SET_PIN_MODE_INPUT(alSensors[si].port, alSensors[si].pin);
    SET_PIN_MODE_OUTPUT(alSensors[si].port, alSensors[si].pin);
    SET_PIN_LOW(alSensors[si].port, alSensors[si].pin);
}

/**
 * @brief Returns state of ambient light sensor
 *
 * @return uint8_t Returns 1 if ambient light sensor detects light else 0
 */
uint8_t al_sensor_light_found(uint8_t alSensorId) {
    // Ensure the sensor ID is valid
    if (ID_INVALID(alSensorId)) {
        return 255;
    }

    // Return whether light has been detected or not
    uint8_t si = alSensorId - AL_SENSOR_OFFSET;

    if (sensorStatus[si] == DISCONNECTED) {
        return DISCONNECTED;
    }

    return (count[si] >= CUT_OFF_COUNT) ? 1 : 0;
}

void al_sensor_charge_capacitor(uint8_t alSensorId) {
    // Set data line to output and set line high
    uint8_t si = alSensorId - AL_SENSOR_OFFSET;

    SET_PIN_MODE_INPUT(alSensors[si].port, alSensors[si].pin);
    SET_PIN_MODE_OUTPUT(alSensors[si].port, alSensors[si].pin);
    SET_PIN_HIGH(alSensors[si].port, alSensors[si].pin);
}

void al_sensor_check_if_connected(uint8_t alSensorId) {

    uint8_t si = alSensorId - AL_SENSOR_OFFSET;
    // I think after you set the pin high, if you change to an input, the
    // charge still remains on the pin so when you change to an input it
    // will read high if nothing is connected. If you set the pin low for
    // a short period of time (short enough for charge on pin to leave
    // and charge on capacitor to stay) then the input will only read high
    // if the sensor is connected
    SET_PIN_LOW(alSensors[si].port, alSensors[si].pin);
    HAL_Delay(1);

    // Read pin to determine if the sensor is connected or not
    SET_PIN_MODE_INPUT(alSensors[si].port, alSensors[si].pin);
    HAL_Delay(1);
    if (PIN_IS_HIGH(alSensors[si].port, alSensors[si].pin)) {
        sensorStatus[si] = CONNECTED;
    } else {
        sensorStatus[si] = DISCONNECTED;
    }
}

void al_sensor_process_internal_flags(void) {

    if (ambientLightSensorFlag & (0x01 << 0)) {
        al_sensor_discharge_capacitor(AL_SENSOR_1_ID);
        ambientLightSensorFlag &= ~(0x01 << 0);
    }

    if (ambientLightSensorFlag & (0x01 << 1)) {
        al_sensor_record_ambient_light(AL_SENSOR_1_ID);
        ambientLightSensorFlag &= ~(0x01 << 1);
    }

    if (ambientLightSensorFlag & (0x01 << 2)) {
        al_sensor_read_capacitor(AL_SENSOR_1_ID);
        ambientLightSensorFlag &= ~(0x01 << 2);
    }

    if (ambientLightSensorFlag & (0x01 << 3)) {
        al_sensor_charge_capacitor(AL_SENSOR_1_ID);
        ambientLightSensorFlag &= ~(0x01 << 3);
    }

    if (ambientLightSensorFlag & (0x01 << 4)) {
        al_sensor_check_if_connected(AL_SENSOR_1_ID);
        ambientLightSensorFlag &= ~(0x01 << 4);
    }

    if (ambientLightSensorFlag & (0x01 << 5)) {
        al_sensor_discharge_capacitor(AL_SENSOR_2_ID);
        ambientLightSensorFlag &= ~(0x01 << 5);
    }

    if (ambientLightSensorFlag & (0x01 << 6)) {
        al_sensor_record_ambient_light(AL_SENSOR_2_ID);
        ambientLightSensorFlag &= ~(0x01 << 6);
    }

    if (ambientLightSensorFlag & (0x01 << 7)) {
        al_sensor_read_capacitor(AL_SENSOR_2_ID);
        ambientLightSensorFlag &= ~(0x01 << 7);
    }

    if (ambientLightSensorFlag & (0x01 << 8)) {
        al_sensor_charge_capacitor(AL_SENSOR_2_ID);
        ambientLightSensorFlag &= ~(0x01 << 8);
    }

    if (ambientLightSensorFlag & (0x01 << 9)) {
        al_sensor_check_if_connected(AL_SENSOR_2_ID);
        ambientLightSensorFlag &= ~(0x01 << 9);
    }
}