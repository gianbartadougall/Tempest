/**
 * @file interrupts_config.h
 * @author Gian Barta-Dougall
 * @brief Defines for the priorities of interrupts used within the project. I have not needed to seriously
 * consider priorities for different interrupts so this current implementation may not be suitable for more
 * complicated projects
 * @version 0.1
 * @date 2022-10-26
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef INTERRUPTS_CONFIG_H
#define INTERRUPTS_CONFIG_H

// Maximum priority set to 10 to ensure internal STM32 priority stuff
// is not overriden
#define MAX_PRIORITY 10
#define PRIORITY_1   (1 + MAX_PRIORITY)
#define PRIORITY_2   (2 + MAX_PRIORITY)
#define PRIORITY_3   (3 + MAX_PRIORITY)
#define PRIORITY_4   (4 + MAX_PRIORITY)
#define PRIORITY_5   (5 + MAX_PRIORITY)

/* Configuration for EXTI interrupt priorities */

#define EXTI0_ISR_PRIORITY     PRIORITY_5
#define EXTI1_ISR_PRIORITY     PRIORITY_5
#define EXTI2_ISR_PRIORITY     PRIORITY_5
#define EXTI3_ISR_PRIORITY     PRIORITY_5
#define EXTI4_ISR_PRIORITY     PRIORITY_5
#define EXTI9_5_ISR_PRIORITY   PRIORITY_5
#define EXTI15_10_ISR_PRIORITY PRIORITY_5

/* Configuration for TIM interrupt priorities */

#define TIM1_ISR_PRIORITY PRIORITY_5
#define TIM2_ISR_PRIORITY PRIORITY_5
#define TIM3_ISR_PRIORITY PRIORITY_5
#define TIM6_ISR_PRIORITY PRIORITY_5
#define TIM7_ISR_PRIORITY PRIORITY_5

// Timer 15 runs the task scheduler, important that this priority is higher
// than any interrupt that could pass that task scheduler a task
#define TIM15_ISR_PRIORITY         PRIORITY_1
#define TIM1_UP_TIM16_ISR_PRIORITY PRIORITY_5

#endif // INTERRUPTS_CONFIG_H