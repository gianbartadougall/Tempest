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

#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"

void timer_ms_init(void);
void timer_ms_enable(void);
void timer_ms_disable(void);

#endif // TIMER_MS_H