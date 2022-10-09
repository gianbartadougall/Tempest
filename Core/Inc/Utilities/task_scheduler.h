/**
 * @file task_scheduler.h
 * @author Gian Barta-Dougall
 * @brief System file for task_scheduler
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

/* Public Includes */
#include "version_config.h"

/* Public STM Includes */
#include "stm32l4xx.h"

/* Public #defines */
#define TASK_SCHEDULER_OFFSET      132
#define TEMPEST_GROUP              (0 + TASK_SCHEDULER_OFFSET)
#define BUTTON_GROUP               (1 + TASK_SCHEDULER_OFFSET)
#define AMBIENT_LIGHT_SENSOR_GROUP (2 + TASK_SCHEDULER_OFFSET)

#define MAX_TASKS_PER_RECIPE 10

/* Public Structures and Enumerations */
typedef struct Recipe {
    const uint8_t id;
    const uint32_t functionIds[MAX_TASKS_PER_RECIPE];
    const uint32_t delays[MAX_TASKS_PER_RECIPE];
    const uint8_t actionOnCompletion;
    const uint8_t numTasks;
    const uint8_t group;
} Recipe;

/* Public Variable Declarations */

/* Public Function Prototypes */

void ts_isr(void);

/**
 * @brief Initialise the system library.
 */
void ts_init(void);
void ts_print_queue(void);
void ts_print_linked_list(void);
void ts_link_task_in_queue(uint8_t qi);
void ts_add_recipe_to_queue(Recipe* recipe);
void ts_add_task_to_queue(Recipe* Recipe, uint8_t ri, uint8_t qi);
void ts_enable_scheduler(void);
uint8_t ts_search_queue_for_recipe(Recipe* recipe);
uint8_t ts_cancel_recipe(uint8_t recipeId);

#if (PROJECT_STATUS == UNIT_TESTS)
void ts_run_unit_tests(void);
#endif

#endif // TASK_SCHEDULER_H
