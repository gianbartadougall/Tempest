/**
 * @file synchronous_interrupts.c
 * @author Gian Barta-Dougall
 * @brief System file for synchronous_interrupts
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "synchronous_interrupts.h"
#include "ambient_light_sensor.h"

/* Private STM Includes */

/* Private #defines */

/* Private Structures and Enumerations */

/* Private Variable Declarations */
uint16_t count = 0;
extern uint32_t ambientLightSensorFlags;

/* Private Function Prototypes */

/* Public Functions */

/**
 * @brief Global interrupt for timer 3
 *
 */
void TIM3_IRQHandler(void) {

    switch (count) {
        case 50:
#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED
            ambientLightSensorFlags |= ALS1_DISCHARGE_CAPACITOR;
            ambientLightSensorFlags |= ALS2_DISCHARGE_CAPACITOR;
#endif
            break;
        case 350:
#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED
            ambientLightSensorFlags |= ALS1_RECORD_AMBIENT_LIGHT;
            ambientLightSensorFlags |= ALS2_RECORD_AMBIENT_LIGHT;
#endif
            break;
        case 58000:
#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED
            ambientLightSensorFlags |= ALS1_READ_AMBIENT_LIGHT;
            ambientLightSensorFlags |= ALS2_READ_AMBIENT_LIGHT;
#endif
            break;
        case 59000:
#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED
            ambientLightSensorFlags |= ALS1_CHARGE_CAPACITOR;
            ambientLightSensorFlags |= ALS2_CHARGE_CAPACITOR;
#endif
            break;
        case 60000:
#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED
            ambientLightSensorFlags |= ALS1_CONFIRM_CONNECTION;
            ambientLightSensorFlags |= ALS2_CONFIRM_CONNECTION;
#endif
            break;
        default:
            break;
    }

    count++;
}

/* Private Functions */
