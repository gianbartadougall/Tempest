/**
 * @file task_scheduler.c
 * @author Gian Barta-Dougall
 * @brief System file for task_scheduler
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "utilities.h"
#include "task_scheduler_config.h"
#include "hardware_config.h"

/* Private STM Includes */

/* Private #defines */

#define TS_TIMER           HC_TS_TIMER
#define TS_TIMER_MAX_COUNT HC_TS_TIMER_MAX_COUNT

// Maximum number should be large enough to hold the maximum number of tasks
// that could be queued at any instance in time
#define MAX_NUM_TASKS_IN_QUEUE 12

// Any empty task in the queue will have this function ID. This allows the
// scheduler to know the task is empty. Value can be arbitrary as long as
// it is not a possible functionID. Possible function IDs are 0 - 31
#define TS_NONE 255

/* Private Structures and Enumerations */

typedef struct Task {
    uint8_t recipieId;
    uint8_t functionId;
    uint8_t group;
    uint32_t executionTime;
    struct Task* nextTask;
} Task;

/* Private Variable Declarations */
uint32_t buttonTasksFlag        = 0;
uint32_t tempestTasksFlag       = 0;
uint32_t ambientLightSensorFlag = 0;

// Stores the physical tasks that make up the queue
Task queue[MAX_NUM_TASKS_IN_QUEUE];

// Pointer to first task in queue
Task* headTask;

/* Private Function Prototypes */
uint8_t ts_search_queue_for_empty_index(void);
uint32_t ts_calculate_task_execution_time(Recipe* recipe, uint8_t index);
void ts_add_task_to_queue(Recipe* recipe, uint8_t ri, uint8_t qi);
void ts_add_task_to_queue(Recipe* recipe, uint8_t ri, uint8_t qi);
void ts_print_task(Task* task);
void ts_reset_task(Task* task);
void ts_enable_scheduler(void);
void ts_disable_scheduler(void);
void ts_update_capture_compare(void);
uint8_t ts_remove_recipe(uint8_t recipeId);

/* Public Functions */

void ts_init(void) {

    // No tasks in queue yet
    headTask = NULL;

    // Reset all tasks in queue
    for (uint8_t i = 0; i < MAX_NUM_TASKS_IN_QUEUE; i++) {
        ts_reset_task(&queue[i]);
    }

    // Add recipes that repeat to queue
    for (int i = 0; i < NUM_REPEATED_RECIPES; i++) {
        ts_add_recipe_to_queue(&repeatedRecipes[i]);
    }
}

void ts_enable_scheduler(void) {
    TS_TIMER->EGR |= (TIM_EGR_UG);    // Reset counter to 0 and update all registers
    TS_TIMER->DIER |= TIM_DIER_CC1IE; // Enable interrupts
    TS_TIMER->CR1 |= TIM_CR1_CEN;
}

void ts_disable_scheduler(void) {
    TS_TIMER->DIER &= 0x00;          // Disable all interrupts
    TS_TIMER->CR1 &= ~(TIM_CR1_CEN); // Disbable timer
}

void copy_task_into_queue(uint8_t qi, Recipe* recipe, uint8_t ti) {
    queue[qi].executionTime = ts_calculate_task_execution_time(recipe, ti);
    queue[qi].recipieId     = recipe->id;
    queue[qi].functionId    = recipe->functionIds[ti];
    queue[qi].group         = recipe->group;
    queue[qi].nextTask      = NULL;
}

/**
 * @brief This code is ugly and needs to be updated
 *
 * @param qi
 */
void order_task_in_linked_list(uint8_t qi) {

    // Place new task at front of linked list if it has the soonest execution time
    if (headTask == NULL || queue[qi].executionTime < headTask->executionTime) {
        queue[qi].nextTask = headTask;                // Set next task to be NULL or current first task in linked list
        headTask           = &queue[qi];              // Set the first task in linked list to the new task
        TS_TIMER->CCR1     = headTask->executionTime; // Update ISR execution time
        return;
    }

    // Save the current head
    Task* currentHead;
    currentHead = headTask;

    while (headTask->nextTask != NULL) {
        // Insert task into linked list when the new tasks execution time
        // is less then the ith Tasks execution time
        if (queue[qi].executionTime < headTask->nextTask->executionTime) {
            queue[qi].nextTask = headTask->nextTask;
            break;
        }

        // Increment to next task in linked list
        headTask = headTask->nextTask;
    }

    headTask->nextTask = &queue[qi];
    headTask           = currentHead;
}

void update_isr_execution_time(void) {
    TS_TIMER->CCR1 = headTask->executionTime;
}

void ts_add_recipe_to_queue(Recipe* recipe) {

    uint8_t ti = 0;

    // Copy tasks from new recipe into queue
    for (uint8_t qi = 0; qi < MAX_NUM_TASKS_IN_QUEUE; qi++) {
        if (queue[qi].functionId != TS_NONE) {
            continue;
        }

        // Copy contents from task into queue
        copy_task_into_queue(qi, recipe, ti);

        // Order the new task in the linked list
        order_task_in_linked_list(qi);

        // Increment task
        ti++;

        // Break out of loop when all tasks have been copied across
        if (ti == recipe->numTasks) {
            break;
        }
    }

    // Turn on timer if required
    if ((TS_TIMER->CR1 & TIM_CR1_CEN) == 0) {
        ts_enable_scheduler();
    }
}

/**
 * @brief Cancels a recipe in the queue if the recipe could be found
 *
 * @param recipeId The ID of the recipe to be cancelled
 * @return uint8_t True if the recipe was found and cancelled else false
 */
uint8_t ts_cancel_recipe(uint8_t recipeId) {
    return ts_remove_recipe(recipeId);
}

/* Private Functions */

/**
 * @brief Searches if queue contains tasks from a given recipe
 *
 * @param recipe The recipe who's tasks will be searched for
 * @return uint8_t true if tasks were found else false
 */
uint8_t ts_search_queue_for_recipe(Recipe* recipe) {

    // Return true if a task with the same ID as the given recipe was found
    while (headTask != NULL) {
        if (headTask->recipieId == recipe->id) {
            return TRUE;
        }
    }

    return FALSE;
}

/**
 * @brief Make timer call ISR when the timer count == execution time of
 * the first task in the queue
 */
void ts_update_capture_compare(void) {
    if (headTask->executionTime != TS_NONE) {
        TS_TIMER->CCR1 = headTask->executionTime;
        // char m[60];
        // sprintf(m, "cap compare = %li\tCurrent CNT: %li\r\n", TS_TIMER->CCR1, TS_TIMER->CNT);
        // debug_prints(m);
    }
}

/**
 * @brief Searches queue for a free index that a new task can be
 * assigned to
 *
 * @return uint8_t The free index if one was found else a number
 * greater than the queue length
 */
// uint8_t ts_search_queue_for_empty_index(void) {

//     // Return first free index
//     for (uint8_t i = 0; i < MAX_NUM_TASKS_IN_QUEUE; i++) {
//         if (queue[i].functionId == TS_NONE) {
//             return i;
//         }
//     }

//     // Returing an invalid index tells caller that no free postions
//     // were available
//     return MAX_NUM_TASKS_IN_QUEUE + 1;
// }

/**
 * @brief Copies the values of a task in a recipe to an empty task in the queue
 *
 * @param recipe The recipe containg the task to be copied
 * @param ri The index of the task in the recipe
 * @param qi The index of the empty task in the queue
 */
// void ts_add_task_to_queue(Recipe* recipe, uint8_t ri, uint8_t qi) {

//     // Ensure the empty task is empty
//     if (queue[qi].functionId != TS_NONE) {
//         return;
//     }

//     queue[qi].executionTime = ts_calculate_task_execution_time(recipe, ri);
//     queue[qi].recipieId     = recipe->id;
//     queue[qi].functionId    = recipe->functionIds[ri];
//     queue[qi].group         = recipe->group;
//     queue[qi].nextTask      = NULL;

//     // char m[50];
//     // sprintf(m, "Execution time: %li\r\n", queue[qi].executionTime);
//     // debug_prints(m);
// }

/**
 * @brief Returns the (current timer count + a given delay)
 *
 * @param delayUntilExecution The given delay
 * @return uint32_t The current count + delay
 */
uint32_t ts_calculate_task_execution_time(Recipe* recipe, uint8_t index) {
    uint32_t delayUntilExecution = 0;
    for (uint8_t i = 0; i <= index; i++) {
        delayUntilExecution += recipe->delays[i];
    }

    return (TS_TIMER->CNT + delayUntilExecution) % TS_TIMER_MAX_COUNT;
}

/**
 * @brief Sets all the parameters of a given task in the queue to
 * their default state
 *
 * @param task A pointer to the task in the queue to reset
 */
void ts_reset_task(Task* task) {
    // char m[50];
    // sprintf(m, "%p reset\r\n", task);
    // debug_prints(m);
    task->functionId    = TS_NONE;
    task->recipieId     = TS_NONE;
    task->group         = TS_NONE;
    task->executionTime = TS_NONE;
    task->nextTask      = NULL;
}

/**
 * @brief This code is ugly and needs to be updated
 *
 * @param qi
 */
void ts_link_task_in_queue(uint8_t qi) {

    // Set task as first task in linked list if no other tasks are in the linked list
    if (headTask->functionId == TS_NONE) {
        headTask = &queue[qi];
        return;
    }

    // Set task to first task in linked list if the execution time for the new task
    // is before the execution time of the current head task
    if (queue[qi].executionTime < headTask->executionTime) {
        queue[qi].nextTask = headTask;
        headTask           = &queue[qi];
        // char m[50];
        // sprintf(m, "qi: %i\tTask %p now front. Next task is %p\r\n", qi, &queue[qi], queue[qi].nextTask);
        // debug_prints(m);
        return;
    }

    // Save the current head
    Task* currentHead;
    currentHead = headTask;

    while (headTask->nextTask != NULL) {
        // Insert task into linked list when the new tasks execution time
        // is less then the ith Tasks execution time
        if (queue[qi].executionTime < headTask->nextTask->executionTime) {
            queue[qi].nextTask = headTask->nextTask;
            headTask->nextTask = &queue[qi];
            headTask           = currentHead;
            return;
        }

        // Increment to next task in linked list
        headTask = headTask->nextTask;
    }

    headTask->nextTask = &queue[qi];
    headTask           = currentHead;
}

void ts_isr(void) {
    char m[100];
    // sprintf(m, "ISR Run @t = %li\r\n", TIM15->CNT);
    // debug_prints(m);
    uint32_t executionTime = headTask->executionTime;

    // Set flags for all tasks that have the current execution time
    while (headTask->executionTime == executionTime) {

        switch (headTask->group) {
            case TEMPEST_GROUP:
                tempestTasksFlag |= (0x01 << headTask->functionId);
                break;
            case BUTTON_GROUP:
                // buttonTasksFlag |= (0x01 << headTask->functionId);
                break;
            case AMBIENT_LIGHT_SENSOR_GROUP:
                ambientLightSensorFlag |= (0x01 << headTask->functionId);
                // sprintf(m, "Id: %i\tTIME: %li\r\n", headTask->functionId, ambientLightSensorFlag);
                // debug_prints(m);
                break;
            default:
                break;
        }

        sprintf(m, "Task %p with func id %i ISR finished. Next task: %p with func Id %i\r\n", headTask,
                headTask->functionId, headTask->nextTask, headTask->nextTask->functionId);
        debug_prints(m);

        uint8_t recipeId   = headTask->recipieId;
        uint8_t functionId = headTask->functionId;
        Task* nextTask;
        nextTask = headTask->nextTask;
        ts_reset_task(headTask);
        headTask = nextTask;

        // Add repeat recipe to queue if the task that just executed was the
        // last task in a repeat recipe
        for (uint8_t i = 0; i < NUM_REPEATED_RECIPES; i++) {
            uint8_t lastTaskFunctionId = repeatedRecipes[i].functionIds[repeatedRecipes[i].numTasks - 1];
            if ((recipeId == repeatedRecipes[i].id) && (functionId == lastTaskFunctionId)) {
                debug_prints("Adding tasks to queue!\r\n");
                ts_add_recipe_to_queue(&repeatedRecipes[i]);
            }
        }
    }

    // Disable and reset timer (for power consumption) if there are no more tasks
    // in queue
    if (headTask == NULL) {
        debug_prints("Disabled\r\n");
        ts_disable_scheduler();
        return;
    }

    // Set next ISR to trigger when count reaches the execution timer of the next task
    // in the queue
    ts_update_capture_compare();
}

uint8_t ts_remove_recipe(uint8_t recipeId) {
    // debug_prints("REMOVING RECIPE\r\n");
    Task* currentHead;
    currentHead         = headTask;
    uint8_t taskRemoved = FALSE;

    while (headTask != NULL && headTask->recipieId == recipeId) {
        ts_reset_task(headTask);
        headTask    = headTask->nextTask;
        currentHead = headTask;
        taskRemoved = TRUE;
    }

    if (headTask == NULL) {
        return taskRemoved;
    }

    while (headTask->nextTask != NULL) {
        if (headTask->nextTask->recipieId == recipeId) {
            Task* taskToRemove;
            taskToRemove       = headTask->nextTask;
            headTask->nextTask = headTask->nextTask->nextTask;
            ts_reset_task(taskToRemove);
            taskRemoved = TRUE;
            continue;
        }

        headTask = headTask->nextTask;
    }

    headTask = currentHead;
    return taskRemoved;
}

void ts_print_linked_list(void) {
    debug_prints("---------- LINKED LIST: ----------\r\n");
    Task* currentHead;
    currentHead = headTask;

    while (headTask != NULL) {
        ts_print_task(headTask);
        headTask = headTask->nextTask;
    }

    headTask = currentHead;
    debug_prints("----------              ----------\r\n");
}

void ts_print_task(Task* task) {
    char msg[150];
    sprintf(msg, "Address: %p\tRec ID: %i\tEx time: %li\tfunc ID: %i\tGroup: %i\tnext task: %p\r\n", task,
            task->recipieId, task->executionTime, task->functionId, task->group, task->nextTask);
    debug_prints(msg);
}

void ts_print_queue(void) {
    debug_prints("---------- QUEUE: ----------\r\n");
    for (uint8_t i = 0; i < MAX_NUM_TASKS_IN_QUEUE; i++) {
        char msg[120];
        sprintf(msg, "queue[%i]:\tAddress: %p\tRec ID: %i\tEx time: %li\tfunc ID: %i\tGroup: %i\tnext task: %p\r\n", i,
                &queue[i], queue[i].recipieId, queue[i].executionTime, queue[i].functionId, queue[i].group,
                queue[i].nextTask);
        debug_prints(msg);
    }
    debug_prints("----------        ----------\r\n");
}

/************************************ UNIT TESTING ************************************/

#if (PROJECT_STATUS == UNIT_TESTS)
uint8_t UNIT_TESTS_FAILED = 0;

    #include "assert.h"

    #define ASSERT(test)                                                                            \
        do {                                                                                        \
            if (!(test)) {                                                                          \
                char msg[100];                                                                      \
                sprintf(msg, "Unit test failed in File %s line number %d\r\n", __FILE__, __LINE__); \
                debug_prints(msg);                                                                  \
                UNIT_TESTS_FAILED++;                                                                \
            }                                                                                       \
        } while (0)

/* Function definitions */
void ts_add_recipe_to_queue_test1(void);
void ts_add_recipe_to_queue_test2(void);
void ts_remove_recipe_test1(void);
void ts_remove_recipe_test2(void);

Recipe recipe1 = {
    .delays      = {1000, 1500, 400, 300},
    .functionIds = {3, 4, 2, 1},
    .group       = 0,
    .id          = 0,
    .numTasks    = 4,
};

Recipe recipe2 = {
    .delays      = {500, 400, 2000, 5000},
    .functionIds = {6, 5, 7, 8},
    .group       = 1,
    .id          = 1,
    .numTasks    = 4,
};

Recipe recipe3 = {
    .delays      = {5000, 2000},
    .functionIds = {10, 9},
    .group       = 2,
    .id          = 2,
    .numTasks    = 2,
};

void ts_run_unit_tests(void) {
    debug_prints("STARTING TESTS\r\n");
    ts_add_recipe_to_queue_test1();
    ts_add_recipe_to_queue_test2();
    ts_remove_recipe_test1();
    // remove_recipe_test2();

    char m[50];
    sprintf(m, "UNIT TESTS FAILED: %i\r\n", UNIT_TESTS_FAILED);
    debug_prints(m);
}

void add_recipe_to_queue_test1(void) {

    ts_init();
    ts_add_recipe_to_queue(&recipe1);
    ts_add_recipe_to_queue(&recipe2);
    ts_add_recipe_to_queue(&recipe3);

    // Confirm tasks were placed in correct order in queue
    ASSERT(queue[0].functionId == 3);
    ASSERT(queue[1].functionId == 4);
    ASSERT(queue[2].functionId == 2);
    ASSERT(queue[3].functionId == 1);
    ASSERT(queue[4].functionId == 6);
    ASSERT(queue[5].functionId == 5);
    ASSERT(queue[6].functionId == 7);
    ASSERT(queue[7].functionId == 8);
    ASSERT(queue[8].functionId == 10);
    ASSERT(queue[9].functionId == 9);

    ASSERT(queue[10].functionId == TS_NONE);
    ASSERT(queue[11].functionId == TS_NONE);
}

void add_recipe_to_queue_test2(void) {

    ts_init();
    ts_add_recipe_to_queue(&recipe1);
    ts_add_recipe_to_queue(&recipe2);
    ts_add_recipe_to_queue(&recipe3);
    ts_print_linked_list();
    // Confirm tasks were placed in correct order in linked list
    uint8_t answers[10] = {1, 2, 5, 6, 3, 4, 7, 9, 8, 10};
    for (uint8_t i = 0; i < 10; i++) {
        ASSERT(headTask->functionId == answers[i]);
        headTask = headTask->nextTask;
    }

    ASSERT(headTask == NULL);
}

void remove_recipe_test1(void) {

    ts_init();
    debug_clear();
    ts_add_recipe_to_queue(&recipe1);
    ts_add_recipe_to_queue(&recipe2);
    ts_add_recipe_to_queue(&recipe3);

    ts_remove_recipe(recipe2.id);

    // Confirm tasks from queue were correctly emptied
    ASSERT(queue[0].functionId == 3);
    ASSERT(queue[1].functionId == 4);
    ASSERT(queue[2].functionId == 2);
    ASSERT(queue[3].functionId == 1);

    ASSERT(queue[4].functionId == TS_NONE);
    ASSERT(queue[5].functionId == TS_NONE);
    ASSERT(queue[6].functionId == TS_NONE);
    ASSERT(queue[7].functionId == TS_NONE);

    ASSERT(queue[8].functionId == 10);
    ASSERT(queue[9].functionId == 9);
}

void remove_recipe_test2(void) {

    ts_init();
    ts_add_recipe_to_queue(&recipe1);
    ts_add_recipe_to_queue(&recipe2);
    ts_add_recipe_to_queue(&recipe3);

    ts_remove_recipe(recipe2.id);

    // Confirm tasks were placed in correct order in linked list
    uint8_t answers[6] = {1, 2, 3, 4, 9, 10};
    for (uint8_t i = 0; i < 6; i++) {
        ASSERT(headTask->functionId == answers[i]);
        headTask = headTask->nextTask;
    }

    ASSERT(headTask == NULL);
}

#endif