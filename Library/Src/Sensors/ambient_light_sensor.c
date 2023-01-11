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
#define ID_INVALID(id)            ((id < AL_SENSOR_ID_OFFSET) || (id > (NUM_AL_SENSORS - 1 + AL_SENSOR_ID_OFFSET)))
#define AL_SENSOR_ID_TO_INDEX(id) (id - AL_SENSOR_ID_OFFSET)

#define ASSERT_VALID_ENCODER_ID(id)                                                            \
    do {                                                                                       \
        if ((id < AL_SENSOR_ID_OFFSET) || (id > (NUM_AL_SENSORS - 1 + AL_SENSOR_ID_OFFSET))) { \
            char msg[100];                                                                     \
            sprintf(msg, "Invalid ID: File %s line number %d\r\n", __FILE__, __LINE__);        \
            log_prints(msg);                                                                   \
            return;                                                                            \
        }                                                                                      \
    } while (0)

#define ASSERT_VALID_ENCODER_ID_RETVAL(id, retval)                                             \
    do {                                                                                       \
        if ((id < AL_SENSOR_ID_OFFSET) || (id > (NUM_AL_SENSORS - 1 + AL_SENSOR_ID_OFFSET))) { \
            char msg[100];                                                                     \
            sprintf(msg, "Invalid ID: File %s line number %d\r\n", __FILE__, __LINE__);        \
            log_prints(msg);                                                                   \
            return retval;                                                                     \
        }                                                                                      \
    } while (0)

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

/* Function prototypes */

/**
 * @brief Setting the pin to input allows the capacitor in the circuit to discharge
 * creating a voltage that can be read to determine if there was ambient light
 */
void al_sensor_read_capacitor(uint8_t alSensorId) {
    // Ensure the sensor ID is valid
    ASSERT_VALID_ENCODER_ID(alSensorId);
    uint8_t si = AL_SENSOR_ID_TO_INDEX(alSensorId);

    // char m[60];
    // sprintf(m, "AFTER %d\r\n", PIN_IDR_STATE(alSensor1.port, alSensor1.pin));
    // log_prints(m);
    SET_PIN_MODE_INPUT(alSensors[0].port, alSensors[0].pin);

    // HAL_Delay(1); // Wait 1ms for IDR to update
    // sprintf(m, "AFTER1 %d\r\n", PIN_IDR_STATE(alSensors[0].port, alSensors[0].pin));
    // log_prints(m);

    // Update count
    if (PIN_IDR_IS_HIGH(alSensors[0].port, alSensors[0].pin)) {
        char m[60];
        sprintf(m, "%s PIN WAS HIGH\r\n", si == 0 ? "ALS1" : "ALS2");
        log_prints(m);
        if (count[si] < MAX_COUNT) {
            count[si] = (count[si] == CUT_OFF_COUNT) ? MAX_COUNT : (count[si] + 1);
            log_prints("+1\r\n");
        }
    } else {
        char m[60];
        sprintf(m, "%s PIN WAS LOW\r\n", si == 0 ? "ALS1" : "ALS2");
        log_prints(m);
        if (count[si] > 0) {
            count[si] = (count[si] == CUT_OFF_COUNT) ? 0 : (count[si] - 1);
            log_prints("-1\r\n");
        }
    }
}

/**
 * @brief Setting the pin to analogue mode prevents the capacitor from dishcharging
 * too quickly allowing it to slowly charge based on the amount of ambient light
 */
void al_sensor_record_ambient_light(uint8_t alSensorId) {
    // Ensure the sensor ID is valid
    ASSERT_VALID_ENCODER_ID(alSensorId);
    uint8_t si = AL_SENSOR_ID_TO_INDEX(alSensorId);

    // Confirm the capacitor is fully discharged
    if (PIN_IDR_IS_HIGH(alSensors[si].port, alSensors[si].pin)) {
        char m[60];
        sprintf(m, "%s IDR WAS HIGH\r\n", si == 0 ? "ALS1" : "ALS2");
        log_prints(m);
    }

    // char m[60];
    // sprintf(m, "BEFORE %d\r\n", PIN_IDR_STATE(alSensor1.port, alSensor1.pin));
    // log_prints(m);
    SET_PIN_MODE_ANALOGUE(alSensors[si].port, alSensors[si].pin);
}

/**
 * @brief The sensor is reset by fully discharging the capacitor. This can be done by setting the
 * data pin low
 */
void al_sensor_discharge_capacitor(uint8_t alSensorId) {
    // Ensure the sensor ID is valid
    ASSERT_VALID_ENCODER_ID(alSensorId);
    uint8_t si = AL_SENSOR_ID_TO_INDEX(alSensorId);

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
    ASSERT_VALID_ENCODER_ID_RETVAL(alSensorId, 255);
    uint8_t si = AL_SENSOR_ID_TO_INDEX(alSensorId);

    // Return whether light has been detected or not
    if (alSensors[si].status == DISCONNECTED) {
        return DISCONNECTED;
    }

    return (count[si] >= CUT_OFF_COUNT) ? 1 : 0;
}

void al_sensor_charge_capacitor(uint8_t alSensorId) {

    ASSERT_VALID_ENCODER_ID(alSensorId);
    uint8_t si = AL_SENSOR_ID_TO_INDEX(alSensorId);

    // Set data line to output and set line high
    SET_PIN_MODE_INPUT(alSensors[si].port, alSensors[si].pin);
    SET_PIN_MODE_OUTPUT(alSensors[si].port, alSensors[si].pin);
    SET_PIN_HIGH(alSensors[si].port, alSensors[si].pin);
}

uint8_t al_sensor_status(uint8_t alSensorId) {
    ASSERT_VALID_ENCODER_ID_RETVAL(alSensorId, DISCONNECTED);
    uint8_t si = AL_SENSOR_ID_TO_INDEX(alSensorId);

    return alSensors[si].status;
}

void al_sensor_check_if_connected(uint8_t alSensorId) {

    ASSERT_VALID_ENCODER_ID(alSensorId);
    uint8_t si = AL_SENSOR_ID_TO_INDEX(alSensorId);

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
    if (PIN_IDR_IS_HIGH(alSensors[si].port, alSensors[si].pin)) {
        alSensors[si].status = CONNECTED;
        log_prints("connected\r\n");
    } else {
        alSensors[si].status = DISCONNECTED;
        log_prints("disconnected\r\n");
    }
}

void al_sensor_process_internal_flags(void) {

    // Skip recording ambient light if the sensor is not connected
    if (alSensors[AL_SENSOR_ID_TO_INDEX(AL_SENSOR_1_ID)].status == CONNECTED) {
        if (FLAG_IS_SET(ambientLightSensorFlag, ALS1_DISCHARGE_CAPACITOR)) {
            FLAG_CLEAR(ambientLightSensorFlag, ALS1_DISCHARGE_CAPACITOR);
            log_prints("Discharging 1\r\n");
            al_sensor_discharge_capacitor(AL_SENSOR_1_ID);
        }

        if (FLAG_IS_SET(ambientLightSensorFlag, ALS1_RECORD_AMBIENT_LIGHT)) {
            FLAG_CLEAR(ambientLightSensorFlag, ALS1_RECORD_AMBIENT_LIGHT);
            log_prints("Recording light 1\r\n");
            al_sensor_record_ambient_light(AL_SENSOR_1_ID);
        }

        if (FLAG_IS_SET(ambientLightSensorFlag, ALS1_READ_AMBIENT_LIGHT)) {
            FLAG_CLEAR(ambientLightSensorFlag, ALS1_READ_AMBIENT_LIGHT);
            log_prints("Reading light level 1\r\n");
            al_sensor_read_capacitor(AL_SENSOR_1_ID);
        }
    }

    if (FLAG_IS_SET(ambientLightSensorFlag, ALS1_CHARGE_CAPACITOR)) {
        FLAG_CLEAR(ambientLightSensorFlag, ALS1_CHARGE_CAPACITOR);
        log_prints("Charging cap 1\r\n");
        al_sensor_charge_capacitor(AL_SENSOR_1_ID);
    }

    if (FLAG_IS_SET(ambientLightSensorFlag, ALS1_CONFIRM_CONNECTION)) {
        FLAG_CLEAR(ambientLightSensorFlag, ALS1_CONFIRM_CONNECTION);
        log_prints("Confirming connection 1\r\n");
        al_sensor_check_if_connected(AL_SENSOR_1_ID);
    }

    // if (alSensors[AL_SENSOR_ID_TO_INDEX(AL_SENSOR_2_ID)].status == CONNECTED) {
    //     if (FLAG_IS_SET(ambientLightSensorFlag, ALS2_DISCHARGE_CAPACITOR)) {
    //         FLAG_CLEAR(ambientLightSensorFlag, ALS2_DISCHARGE_CAPACITOR);
    //         al_sensor_discharge_capacitor(AL_SENSOR_2_ID);
    //     }

    //     if (FLAG_IS_SET(ambientLightSensorFlag, ALS2_RECORD_AMBIENT_LIGHT)) {
    //         FLAG_CLEAR(ambientLightSensorFlag, ALS2_RECORD_AMBIENT_LIGHT);
    //         al_sensor_record_ambient_light(AL_SENSOR_2_ID);
    //     }

    //     if (FLAG_IS_SET(ambientLightSensorFlag, ALS2_READ_AMBIENT_LIGHT)) {
    //         FLAG_CLEAR(ambientLightSensorFlag, ALS2_READ_AMBIENT_LIGHT);
    //         al_sensor_read_capacitor(AL_SENSOR_2_ID);
    //     }
    // }

    // if (FLAG_IS_SET(ambientLightSensorFlag, ALS2_CHARGE_CAPACITOR)) {
    //     FLAG_CLEAR(ambientLightSensorFlag, ALS2_CHARGE_CAPACITOR);
    //     al_sensor_charge_capacitor(AL_SENSOR_2_ID);
    // }

    // if (FLAG_IS_SET(ambientLightSensorFlag, ALS2_CONFIRM_CONNECTION)) {
    //     FLAG_CLEAR(ambientLightSensorFlag, ALS2_CONFIRM_CONNECTION);
    //     al_sensor_check_if_connected(AL_SENSOR_2_ID);
    // }
}