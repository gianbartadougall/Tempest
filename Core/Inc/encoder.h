#ifndef ENCODER_H
#define ENCODER_H

#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"

#define ENCODER_AT_MIN_DISTANCE 0
#define ENCODER_AT_MAX_DISTANCE 1

void encoder_init(void);
void encoder_isr(void);
uint32_t encoder_distance_travelled(void);
void encoder_set_direction_positive(void);
void encoder_set_direction_negative(void);
uint8_t encoder_at_minimum_distance(void);
uint8_t encoder_at_maximum_distance(void);
uint32_t encoder_get_count(void);
void encoder_reset(void);
void encoder_print_state(void);
void encoder_update(void);
void encoder_disable(void);

void encoder_enter_manual_override(void);
void encoder_exit_manual_override(void);

void encoder_isr_reset_min_value(void);
void encoder_isr_reset_max_value(void);

#endif // ENCODER_H