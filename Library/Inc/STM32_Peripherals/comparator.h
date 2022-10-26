/**
 * @file comparator.h
 * @author Gian Barta-Dougall
 * @brief Peripheral driver for the STM32L432KC internal comparators
 * @version 0.1
 * @date 2022-06-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef COMPARATOR_H
#define COMPARATOR_H

#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_comp.h"

void comparator_init(void);

#endif // COMPARATOR_H