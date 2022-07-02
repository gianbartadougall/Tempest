#ifndef TEMPEST_H
#define TEMPEST_H

#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"

#include "encoder.h"
#include "ambient_light_sensor.h"
#include "pushbutton.h"
#include "motor_driver.h"
#include "board.h"
#include "debug_log.h"
#include "piezo_buzzer.h"
#include "comparator.h"
#include "flag.h"

void tempest_update_system_state(void);
void tempest_update_motor_state(void);
void tempest_update_mode(void);
void tempest_update_mode_indicator(void);
void tempest_hardware_init(void);

void tempest_print_system_state(void);
void tempest_print_motor_state(void);

void tempest_isr_set_manual_override(void);
void tempest_isr_clear_manual_override(void);

void tempest_isr_encoder_at_min_value(void);
void tempest_isr_encoder_at_max_value(void);

#define MANUAL_OVERRIDE_FLAG FLAG_0

#endif // TEMPEST_H

