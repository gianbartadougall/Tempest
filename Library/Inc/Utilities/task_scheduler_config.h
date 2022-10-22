// #ifndef TASK_SCHEDULER_CONFIG_H
// #define TASK_SCHEDULER_CONFIG_H

// #include "task_scheduler.h"

// /* Includes for function pointers used by recipes that repeat */

// /* Configration for recipes that need to be repeated on a consistent basis.
//  *
//  *
//  */

// /* Includes for repeated recipes */
// #define NUM_REPEATED_RECIPES 1

// /**
//  * @brief To read the ambient light sensor the following steps need to
//  * occur
//  *      1. Set the data line low to discharge capacitor
//  *      2. Delay 1 second to ensure capacitor is discharged
//  *      3. Set data line to high impedence
//  *      4. Delay 59 seconds to let capacitor charge from ambient light
//  *      5. Set data line to input mode
//  *      6. Wait 1ms for IDR to update
//  *      6. Update state based on the IDR of the data pin
//  *      7. Set data line to high
//  *      8. Wait for 1 second for capacitor to charge
//  *      9. Update state based on input (if sensor is connected input = 1 else 0)
//  */
// const Recipe readAmbientLightSensor1 = {
//     .delays      = {50, 300, 55000, 20, 300},
//     .functionIds = {0, 1, 2, 3, 4},
//     .group       = AMBIENT_LIGHT_SENSOR_GROUP,
//     .id          = TS_ID_READ_AL_SENSOR_1,
//     .numTasks    = 5,
// };

// const Recipe readAmbientLightSensor2 = {
//     .delays      = {50, 300, 55000, 20, 300},
//     .functionIds = {5, 6, 7, 8, 9},
//     .group       = AMBIENT_LIGHT_SENSOR_GROUP,
//     .id          = TS_ID_READ_AL_SENSOR_2,
//     .numTasks    = 5,
// };

// Recipe repeatedRecipes[NUM_REPEATED_RECIPES] = {readAmbientLightSensor2};

// #endif // TASK_SCHEDULER_CONFIG_H