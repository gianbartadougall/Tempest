/**
 * @file unit_tests.c
 * @author Gian Barta-Dougall
 * @brief System file for unit_tests
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "utilities.h"
#include "unit_tests.h"
#include "task_scheduler.h"

/* Private STM Includes */

/* Private #defines */

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */

/* Public Functions */

void unit_tests_init(void) {

    debug_clear();

    /* Run unit tests for each module */
    ts_run_unit_tests(); // Task scheduler
}

/* Private Functions */
