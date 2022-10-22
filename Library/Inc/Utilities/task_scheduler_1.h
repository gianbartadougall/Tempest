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
#ifndef TASK_SCHEDULER_1_H
#define TASK_SCHEDULER_1_H

/* Public Includes */
#include "version_config.h"

/* Public STM Includes */
#include "stm32l4xx.h"

/* Public #defines */
#define TASK_SCHEDULER_OFFSET      132
#define TEMPEST_GROUP              (0 + TASK_SCHEDULER_OFFSET)
#define BUTTON_GROUP               (1 + TASK_SCHEDULER_OFFSET)
#define AMBIENT_LIGHT_SENSOR_GROUP (2 + TASK_SCHEDULER_OFFSET)
#define BLIND_MOTOR_GROUP          (3 + TASK_SCHEDULER_OFFSET)

#define MAX_TASKS_PER_RECIPE 10

#define TS_TASK_CANCELLED 1
#define TS_TASK_NOT_FOUND 0

/* Public Structures and Enumerations */
typedef struct Task1 {
    // const uint8_t taskSchedulerId;
    const uint16_t delay;
    const uint8_t functionId;
    const uint8_t group;
    const struct Task1* nextTask;
} Task1;

/* Public Variable Declarations */

/* Public Function Prototypes */

void ts_init(void);
void ts_isr(void);
void ts_process_internal_flags(void);
void ts_add_task_to_queue(const Task1* task);
uint8_t ts_cancel_running_task(const Task1* task);
uint8_t ts_task_is_running(const Task1* task);

/**
 * @brief Initialise the system library.
 */

#if (PROJECT_STATUS == UNIT_TESTS)
void ts_run_unit_tests(void);
#endif

#endif // TASK_SCHEDULER_1_H
