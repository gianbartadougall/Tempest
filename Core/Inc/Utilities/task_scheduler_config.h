#ifndef TASK_SCHEDULER_CONFIG_H
#define TASK_SCHEDULER_CONFIG_H

#include "task_scheduler.h"

/* Includes for function pointers used by recipes that repeat */

/* Configration for recipes that need to be repeated on a consistent basis.
 *
 *
 */

/* Includes for repeated recipes */
#define NUM_REPEATED_RECIPES 0

const Recipe readAmbientLightSensor = {
    .delays      = {58999, 1, 1000},
    .functionIds = {0, 1, 2},
    .group       = TEMPEST_GROUP,
    .id          = 0,
    .numTasks    = 3,
};

Recipe repeatedRecipes[NUM_REPEATED_RECIPES];

#endif // TASK_SCHEDULER_CONFIG_H