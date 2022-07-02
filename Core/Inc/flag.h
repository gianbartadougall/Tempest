/**
 * @file flag.h
 * @author Gian Barta-Dougall
 * @brief Peripheral driver for a generic flag
 * @version 0.1
 * @date 2022-07-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef FLAG_H
#define FLAG_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#define FLAG_0 0

#define FLAG_0_PORT GPIOB
#define FLAG_0_HAL_PIN 5

void flag_init(void);
uint8_t flag_state(uint8_t flag);

#endif // FLAG_H