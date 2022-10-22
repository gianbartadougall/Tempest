/**
 * @file timer_ms.c
 * @author Gian Barta-Dougall
 * @brief Generic millisecond timer
 * @version 0.1
 * @date 2022-07-04
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef TIMER_MS_H
#define TIMER_MS_H

/* STM32 Includes */
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"

/* Public Structs */

#define TIMER_MS_MAX_TASK_SIZE  5
#define TIMER_MS_MAX_QUEUE_SIZE 5

/**
 * @brief Task struct that contains information on delay lengths and ISRs that need
 * to be performed
 *
 * @id: A unique ID that allows tasks to be differentiated from eachother
 * @isrs: A list of ISRs that are called (in order) between the list of delays
 * @delays: A list of delays that are carried out (in order) before calling each isr
 * @size: The number of ISRs/delays that need to be performed to compelte the task
 * @index: The current delay/isr the task is up to
 * @delayremaining: In the case that a task is paused due to another task with a higher
 * priority that needs to be run, the amount of time the current task delay has left
 * is saved such that when it resumes, it continues it's delay from where it paused
 * @priority: The priority level of the task. Tasks with higher priorities will run
 * before tasks with lower priorities. A task with a higher priority will cancel an
 * already running task with a lower priority
 * @actiononprioritycancel: Determines what will happen to the task if it gets
 * cancelled because a higher priority task needs to run
 */
typedef struct TimerMsTask {
    uint8_t id;
    void (*isrs[TIMER_MS_MAX_TASK_SIZE])(void);
    uint16_t delays[TIMER_MS_MAX_TASK_SIZE];
    uint8_t size;
    uint8_t index;
    uint8_t status;
    uint16_t delayRemaining;
    uint8_t priority;
    uint8_t actionOnPriorityCancel;
    uint8_t actionOnCompletion;
} TimerMsTask;

typedef struct TimerTaskStatus {
    uint8_t index;
    uint8_t status;
    uint8_t delayRemaining;
} TimerTaskStatus;

typedef struct TimerTaskSettings {
    uint8_t priority;
    uint8_t actionIfMoreUrgentTask;
    uint8_t actionWhenFinished;
} TimerTaskSettings;

typedef struct TimerMsTask1 {
    const uint8_t id;
    const void (*tasks[TIMER_MS_MAX_TASK_SIZE])(void);
    const uint16_t delays[TIMER_MS_MAX_TASK_SIZE];
    const uint8_t size;
    const TimerTaskSettings settings;
    TimerTaskStatus status;
} TimerMsTask1;

/**
 * @brief Handle for each channel that timer has. This handle stores the tasks that
 * it needs to run in order
 *
 * @queue: The list of tasks that the channel will perform in order
 * @size: The number of tasks in the queue
 */
typedef struct TimerMsChannel {
    TimerMsTask* queue[TIMER_MS_MAX_QUEUE_SIZE];
    uint8_t size;
} TimerMsChannel;

/* Public Enumerations */

enum TaskActionOnCancel { TIMER_MS_TASK_CANCEL, TIMER_MS_TASK_RESET, TIMER_MS_TASK_PAUSE };

enum TaskActionOnCompletion { TIMER_MS_TASK_FINISH, TIMER_MS_TASK_REPEAT };

#define TASK_MS_PRIORITY_1 5
#define TASK_MS_PRIORITY_2 4
#define TASK_MS_PRIORITY_3 3
#define TASK_MS_PRIORITY_4 2
#define TASK_MS_PRIORITY_5 1

#define TIMER_MS_CHANNEL_1 0
#define TIMER_MS_CHANNEL_2 1

/**
 * @brief Initialise the timer hardware. This function does not start the timer
 */
void timer_ms_init(void);

/**
 * @brief Initialise parameters into Timer task struct. This function also assigns
 * an ID to the task
 *
 * @param task The pointer to the task to be initialised
 * @param isrs The list of ISRs that need to be run (in order)
 * @param delays The list of delays to wait between each ISR (in order)
 * @param numberOfDelays The size of the list of delays. Note this should be the
 * same size as the list of ISRs
 * @param actionOnPriorityCancel
 * @param priority
 */
void timer_ms_init_task(TimerMsTask* task, void (*isrs[TIMER_MS_MAX_TASK_SIZE])(void),
                        uint16_t delays[TIMER_MS_MAX_TASK_SIZE], uint8_t numberOfDelays, uint8_t actionOnPriorityCancel,
                        uint8_t priority, uint8_t TaskActionOnCompletion);

void timer_ms_add_task(TimerMsTask* task);

uint8_t timer_ms_task_is_in_queue(TimerMsTask* task);

/**
 * @brief Starts the timer
 */
void timer_ms_enable(void);

/**
 * @brief Stops the counter from updating. Does not cancel any delays that have
 * currently been assigned
 *
 */
void timer_ms_disable(void);

void timer_ms_cancel_task(TimerMsTask* handle);

uint8_t timer_ms_cancel_task1(TimerMsTask* task);

void timer_ms_isr(uint8_t chnl);

void timer_ms_status(void);

#endif // TIMER_MS_H