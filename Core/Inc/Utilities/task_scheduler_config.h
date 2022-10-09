#ifndef TASK_SCHEDULER_CONFIG_H
#define TASK_SCHEDULER_CONFIG_H

#include "task_scheduler.h"

/* Includes for function pointers used by recipes that repeat */

/* Configration for recipes that need to be repeated on a consistent basis.
 *
 *
 */

/* Includes for repeated recipes */
#define NUM_REPEATED_RECIPES 1

/**
 * @brief To read the ambient light sensor the following steps need to
 * occur
 *      1. Reset the AL sensor
 *      2. Delay 1s
 *      3. Start a new sycle
 *      4. Delay 59 seconds
 *      5. Set Al sensor to read mode
 *      6. Wait 1ms
 *      6. Read the AL sensor
 */
const Recipe readAmbientLightSensor = {
    .delays      = {3000, 3000, 3000, 3000},
    .functionIds = {0, 1, 2, 3},
    .group       = AMBIENT_LIGHT_SENSOR_GROUP,
    .id          = 0,
    .numTasks    = 4,
};

Recipe repeatedRecipes[NUM_REPEATED_RECIPES] = {readAmbientLightSensor};

#endif // TASK_SCHEDULER_CONFIG_H