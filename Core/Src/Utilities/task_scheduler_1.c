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
#define QUEUE_SIZE 4
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
    Task1* task;
    uint32_t executionTime;
    uint8_t finished;
} TsTask;

/* Private Variable Declarations */
uint32_t buttonTasksFlag        = 0;
uint32_t tempestTasksFlag       = 0;
uint32_t ambientLightSensorFlag = 0;

TsTask queue[QUEUE_SIZE];
TsTask* queueHead[QUEUE_HEAD_SIZE];

uint8_t finishedTasks = 0;
int numTasksInQueue   = 0;
uint8_t k             = 0;

/* Private Function Prototypes */
uint32_t ts_calculate_execution_time(uint32_t delayUntilExecution);
void ts_copy_task_into_queue(uint8_t qi, Task1* task);
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

#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED
    // Add repeated tasks to scheduler
    ts_add_task_to_queue(&alSensor1DischargeCapacitor);
    ts_add_task_to_queue(&alSensor2DischargeCapacitor);
#endif
}

void ts_add_task_to_queue(Task1* task) {
    // // debug_prints("Adding task\r\n");

    // Confirm this task doesn't already exist in the queue
    for (uint8_t i = 0; i < QUEUE_SIZE; i++) {
        if ((queue[i].task == task)) {
            // debug_prints("SKIPPING ADD\r\n");
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

uint8_t ts_cancel_running_task(Task1* task) {
    uint8_t num = 0;

    for (uint8_t qi = 0; qi < QUEUE_SIZE; qi++) {
        if (queue[qi].task == task) {
            char m[60];
            sprintf(m, "Cancelling func id: %i  ", queue[qi].task->functionId);
            // debug_prints(m);
            queue[qi].executionTime = TS_NONE;
            queue[qi].finished      = TS_NONE;
            queue[qi].task          = NULL;
            numTasksInQueue--;
            // ts_remove_task(qi);
            ts_update_first_in_queue();
            num++;
        }
    }

    if (num == 1) {
        return TASK_CANCELLED;
    } else if (num == 0) {
        return TASK_NOT_FOUND;
    }

    // char m[60];
    // sprintf(m, "More than one task of id %i found\r\n", task->functionId);
    // debug_prints(m);

    return TASK_CANCELLED;
}

void ts_process_flags(void) {

    if (finishedTasks == 0) {
        return;
    }

    for (uint8_t qi = 0; qi < QUEUE_SIZE; qi++) {

        if (queue[qi].finished != TASK_FINISHED) {
            // // debug_prints("CONTINUING\r\n");
            continue;
        }

        if (queue[qi].task->nextTask == NULL) {
            ts_remove_task(qi);
            // // debug_prints("removing task\r\n");
        } else {
            // char m[40];
            // sprintf(m, "Moving to task %i\r\n", queue[qi].task->nextTask->functionId);
            // // debug_prints(m);
            ts_move_to_next_task(qi);
        }

        finishedTasks--;
        // char m[40];
        // sprintf(m, "-- Finished tasks now = %i\r\n", finishedTasks);
        // // debug_prints(m);
        if (finishedTasks == 0) {
            break;
        }
    }

    if (finishedTasks != 0) {
        // debug_prints("Error, not all tasks were dealt with!\r\n");

        for (uint8_t i = 0; i < QUEUE_SIZE; i++) {
            if (queue[i].finished == TASK_FINISHED) {
                char m[60];
                sprintf(m, "TASK NOT PROCESSED %i\r\n", queue[i].task->functionId);
                // debug_prints(m);
            }
        }

        finishedTasks = 0;
    }

    // char m[50];
    // sprintf(m, "FINISHED TASKS %i\r\n", finishedTasks);
    // // debug_prints(m);
    ts_update_first_in_queue();
}

void ts_isr(void) {
    // char m[50];
    // sprintf(m, "ISR called @T=%li\r\n", TIM15->CNT);
    // // debug_prints(m);
    uint8_t index = 0;
    TsTask* head  = queueHead[0];

    while (head != NULL) {

        switch (head->task->group) {
            case TEMPEST_GROUP:
                tempestTasksFlag |= (0x01 << head->task->functionId);
                break;
            case BUTTON_GROUP:
                buttonTasksFlag |= (0x01 << head->task->functionId);
                char m[40];
                sprintf(m, "ISR @ Id: %i\tTIME: %li\r\n", head->task->functionId, TS_TIMER->CNT);
                // debug_prints(m);
                // // debug_prints("A  ");
                break;
            case AMBIENT_LIGHT_SENSOR_GROUP:
                ambientLightSensorFlag |= (0x01 << head->task->functionId);
                break;
            default:
                break;
        }
        head->finished = TASK_FINISHED;
        finishedTasks++;
        // char m[60];
        // sprintf(m, "++ Func Id: %i now finished -> FTs = %i\r\n", head->task->functionId, finishedTasks);
        // // debug_prints(m);
        head = queueHead[++index];
    }
}

/* Private Functions */

uint32_t ts_calculate_execution_time(uint32_t delayUntilExecution) {
    return (TS_TIMER->CNT + delayUntilExecution) % TS_TIMER_MAX_COUNT;
}

void ts_copy_task_into_queue(uint8_t qi, Task1* task) {
    // // debug_prints("Copying to queue\r\n");
    queue[qi].task          = task;
    queue[qi].executionTime = ts_calculate_execution_time(task->delay);
    queue[qi].finished      = 0;
    numTasksInQueue++;
}

void ts_remove_task(uint8_t qi) {
    // char m[60];
    // sprintf(m, "Removing func id: %i  ", queue[qi].task->functionId);
    // // debug_prints(m);

    queue[qi].executionTime = TS_NONE;
    queue[qi].finished      = TS_NONE;
    queue[qi].task          = NULL;
    numTasksInQueue--;

    if (numTasksInQueue < 0) {
        // debug_prints("FATAL ERROR - TASKS IN QUEUE WAS NEGATIVE\r\n");
    }
}

uint8_t ts_task_is_running(Task1* task) {
    for (uint8_t qi = 0; qi < QUEUE_SIZE; qi++) {
        if (queue[qi].task == task) {
            return TRUE;
        }
    }

    return FALSE;
}

void ts_move_to_next_task(uint8_t qi) {
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
        // // debug_prints("Disabled\r\n");
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
    // debug_prints(m);
    ts_enable();
}

void ts_enable(void) {
    if ((TS_TIMER->CR1 & TIM_CR1_CEN) == 0) {
        TS_TIMER->EGR |= (TIM_EGR_UG);    // Reset counter to 0 and update all registers
        TS_TIMER->DIER |= TIM_DIER_CC1IE; // Enable interrupts
        TS_TIMER->CR1 |= TIM_CR1_CEN;
    }
}

void ts_disable(void) {
    TS_TIMER->DIER &= 0x00;          // Disable all interrupts
    TS_TIMER->CR1 &= ~(TIM_CR1_CEN); // Disbable timer
}