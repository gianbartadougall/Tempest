/**
 * @file task_scheduler_1.c
 * @author Gian Barta-Dougall
 * @brief System file for task_scheduler_1
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "task_scheduler_1_config.h"
#include "hardware_config.h"
#include "utilities.h"

/* Private STM Includes */

/* Private #defines */
#define QUEUE_SIZE 6
// Plus 1 to ensure that the last item can always be NULL as this allows the isr function to know when
// to stop executing tasks from the queue head list
#define QUEUE_HEAD_SIZE    (QUEUE_SIZE + 1)
#define TS_TIMER           HC_TS_TIMER
#define TS_TIMER_MAX_COUNT HC_TS_TIMER_MAX_COUNT
#define TS_NONE            255
#define TASK_FINISHED      62

/* Private Structures and Enumerations */

// This struct isn't necessary but it means you don't have to copy
// as many items across when adding something into the queue thus can
// run faster
typedef struct TsTask {
    const Task1* task;
    uint32_t executionTime;
    uint8_t finished;
} TsTask;

/* Private Variable Declarations */
uint32_t buttonTasksFlag        = 0;
uint32_t tempestTasksFlag       = 0;
uint32_t blindTasksFlag         = 0;
uint32_t ambientLightSensorFlag = 0;
uint32_t blindMotorFlag         = 0;

TsTask queue[QUEUE_SIZE];
TsTask* queueHead[QUEUE_HEAD_SIZE];

uint8_t finishedTasks = 0;
int numTasksInQueue   = 0;
uint8_t k             = 0;

/* Private Function Prototypes */
uint32_t ts_calculate_execution_time(uint32_t delayUntilExecution);
void ts_copy_task_into_queue(uint8_t qi, const Task1* task);
void ts_remove_task(uint8_t qi);
void ts_move_to_next_task(uint8_t qi);
void ts_update_first_in_queue(void);
void ts_enable(void);
void ts_disable(void);

/* Public Functions */

void ts_init(void) {

    for (uint8_t qi = 0; qi < QUEUE_HEAD_SIZE; qi++) {
        queueHead[qi] = NULL;
    }
}

void ts_add_task_to_queue(const Task1* task) {
    // log_prints("Adding task\r\n");

    // Confirm this task doesn't already exist in the queue
    for (uint8_t i = 0; i < QUEUE_SIZE; i++) {
        if ((queue[i].task == task)) {
            // log_prints("SKIPPING ADD\r\n");
            return;
        } else {
            break;
        }
    }

    for (uint8_t i = 0; i < QUEUE_SIZE; i++) {

        if (queue[i].task != NULL) {
            continue;
        }

        ts_copy_task_into_queue(i, task);
        ts_update_first_in_queue();
        break;
    }
}

uint8_t ts_cancel_running_task(const Task1* task) {
    uint8_t num = 0;

    for (uint8_t qi = 0; qi < QUEUE_SIZE; qi++) {
        // char m[60];
        // sprintf(m, "Test: %p == %p\r\n", queue[qi].task, task);
        // log_prints(m);

        if (queue[qi].task == task) {
            ts_remove_task(qi);
            ts_update_first_in_queue();
            num++;
        }
    }

    if (num == 1) {
        return TS_TASK_CANCELLED;
    } else if (num == 0) {
        // log_prints("task could not be found\r\n");
        return TS_TASK_NOT_FOUND;
    }

    // char m[60];
    // sprintf(m, "More than one task of id %i found\r\n", task->functionId);
    // log_prints(m);

    return TS_TASK_CANCELLED;
}

void ts_process_internal_flags(void) {

    if (finishedTasks == 0) {
        return;
    }

    for (uint8_t qi = 0; qi < QUEUE_SIZE; qi++) {

        if (queue[qi].finished != TASK_FINISHED) {
            continue;
        }

        if (queue[qi].task->nextTask == NULL) {
            ts_remove_task(qi);
        } else {
            ts_move_to_next_task(qi);
        }

        finishedTasks--;

        if (finishedTasks == 0) {
            break;
        }
    }

    if (finishedTasks != 0) {
        // log_prints("Error, not all tasks were dealt with!\r\n");

        for (uint8_t i = 0; i < QUEUE_SIZE; i++) {
            if (queue[i].finished == TASK_FINISHED) {
                char m[60];
                sprintf(m, "TASK NOT PROCESSED %p\r\n", queue[i].task);
                log_prints(m);
            }
        }

        finishedTasks = 0;
    }

    ts_update_first_in_queue();
}

void ts_isr(void) {
    // char m[50];
    // sprintf(m, "ISR called @T=%li\r\n", TIM15->CNT);
    // // log_prints(m);
    uint8_t index = 0;
    TsTask* head  = queueHead[0];

    while (head != NULL) {

        switch (head->task->group) {
            case TEMPEST_GROUP:
                FLAG_SET(tempestTasksFlag, head->task->functionId);
                break;
            case BUTTON_GROUP:
                FLAG_SET(buttonTasksFlag, head->task->functionId);
                break;
            case AMBIENT_LIGHT_SENSOR_GROUP:
                FLAG_SET(ambientLightSensorFlag, head->task->functionId);
                break;
            case BLIND_MOTOR_GROUP:
                FLAG_SET(blindMotorFlag, head->task->functionId);
                break;
            case BLIND_GROUP:
                FLAG_SET(blindTasksFlag, head->task->functionId);
                break;
            default:
                break;
        }
        head->finished = TASK_FINISHED;
        finishedTasks++;
        // char m[60];
        // sprintf(m, "++ Func Id: %i now finished -> FTs = %i\r\n", head->task->functionId, finishedTasks);
        // // log_prints(m);
        head = queueHead[++index];
    }
}

/* Private Functions */

uint32_t ts_calculate_execution_time(uint32_t delayUntilExecution) {
    return (TS_TIMER->CNT + delayUntilExecution) % TS_TIMER_MAX_COUNT;
}

void ts_copy_task_into_queue(uint8_t qi, const Task1* task) {
    // // log_prints("Copying to queue\r\n");
    queue[qi].task          = task;
    queue[qi].executionTime = ts_calculate_execution_time(task->delay);
    queue[qi].finished      = 0;
    numTasksInQueue++;
}

void ts_remove_task(uint8_t qi) {
    // char m[60];
    // sprintf(m, "REMOVING TASK %p\r\n", queue[qi].task);
    // log_prints(m);

    queue[qi].executionTime = TS_NONE;
    queue[qi].finished      = TS_NONE;
    queue[qi].task          = NULL;
    numTasksInQueue--;

    if (numTasksInQueue < 0) {
        // log_prints("FATAL ERROR - TASKS IN QUEUE WAS NEGATIVE\r\n");
    }
}

uint8_t ts_task_is_running(const Task1* task) {
    for (uint8_t qi = 0; qi < QUEUE_SIZE; qi++) {
        if (queue[qi].task == task) {
            return TRUE;
        }
    }

    return FALSE;
}

void ts_move_to_next_task(uint8_t qi) {
    // char m[60];
    // sprintf(m, "MOVING ONTO NEXT TASK %p\r\n", queue[qi].task);
    // log_prints(m);

    queue[qi].task          = queue[qi].task->nextTask;
    queue[qi].executionTime = ts_calculate_execution_time(queue[qi].task->delay);
    queue[qi].finished      = 0;
}

void ts_update_first_in_queue(void) {

    // This ensures the queue head is empty. If this is not here
    // then the tasks will accumulate in this queue and you will get
    // ISRS running that shouldn't
    for (uint8_t i = 0; i < QUEUE_HEAD_SIZE; i++) {
        queueHead[i] = NULL;
    }

    uint32_t currentTime = TS_TIMER->CNT;
    uint32_t zeroPoint   = 0;

    for (uint8_t qi = 0; qi < QUEUE_SIZE; qi++) {
        if (queue[qi].task != NULL && queue[qi].executionTime > currentTime) {
            zeroPoint = currentTime;
        }
    }

    uint8_t queueHeadFound    = 0;
    uint32_t soonestTimeFound = TS_TIMER_MAX_COUNT;

    for (uint8_t qi = 0; qi < QUEUE_SIZE; qi++) {
        if (queue[qi].task == NULL) {
            continue;
        }

        if ((queue[qi].executionTime > zeroPoint) && (queue[qi].executionTime < soonestTimeFound)) {
            soonestTimeFound = queue[qi].executionTime;
            queueHeadFound   = 1;
        }
    }

    if (queueHeadFound == 0) {
        // // log_prints("Disabled\r\n");
        // ts_disable();
        return;
    }

    // Loop through queue and add all tasks that have the same execution time to list
    uint8_t index = 0;
    for (uint8_t qi = 0; qi < QUEUE_SIZE; qi++) {
        if (queue[qi].task == NULL) {
            continue;
        }

        if (queue[qi].executionTime == soonestTimeFound) {
            queueHead[index] = &queue[qi];
            index++;
        }
    }

    // Update capture compare
    TS_TIMER->CCR1 = queueHead[0]->executionTime;

    char m[50];
    sprintf(m, "  Next ISR @T=%li\r\n", TS_TIMER->CCR1);
    // log_prints(m);
    ts_enable();
}

void ts_enable(void) {
    // Only enable timer if its currently disabled
    if ((TS_TIMER->CR1 & TIM_CR1_CEN) == 0) {
        TS_TIMER->EGR |= (TIM_EGR_UG);    // Reset counter to 0 and update all registers
        TS_TIMER->DIER |= TIM_DIER_CC1IE; // Enable interrupts
        TS_TIMER->CR1 |= TIM_CR1_CEN;     // Start the timer
    }
}

void ts_disable(void) {
    TS_TIMER->DIER &= 0x00;          // Disable all interrupts
    TS_TIMER->CR1 &= ~(TIM_CR1_CEN); // Disbable timer
}