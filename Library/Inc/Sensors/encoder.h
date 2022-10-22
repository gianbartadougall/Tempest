#ifndef ENCODER_H
#define ENCODER_H

#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"

#define ENCODER_1_LIMIT_REACHED (0x01 << 0)
#define ENCODER_2_LIMIT_REACHED (0x01 << 1)

#define ENCODER_ID_OFFSET 23
#define ENCODER_1_ID      (0 + ENCODER_ID_OFFSET)
#define ENCODER_2_ID      (1 + ENCODER_ID_OFFSET)

void encoder_init(void);

uint32_t encoder_get_count(uint8_t encoderId);

void encoder_set_direction_up(uint8_t encoderId);
void encoder_set_direction_down(uint8_t encoderId);

void encoder_limit_reached_isr(uint8_t encoderId);
void encoder_disable_limits(uint8_t encoderId);
uint8_t encoder_limits_are_valid(uint8_t encoderId);

uint8_t encoder_at_min_height(uint8_t encoderId);
uint8_t encoder_at_max_height(uint8_t encoderId);

void encoder_enable(uint8_t encoderId);
void encoder_disable(uint8_t encoderId);

uint8_t encoder_probe_connection(uint8_t encoderId);
uint8_t encoder_get_state(uint8_t encoderId);

void encoder_set_lower_bound_interrupt(uint8_t encoderId);
void encoder_set_upper_bound_interrupt(uint8_t encoderId);
uint32_t encoder_get_lower_bound_interrupt(uint8_t encoderId);
uint32_t encoder_get_upper_bound_interrupt(uint8_t encoderId);
void encoder_enable_interrupts(uint8_t encoderId);
void encoder_disable_interrupts(uint8_t encoderId);
#endif // ENCODER_H