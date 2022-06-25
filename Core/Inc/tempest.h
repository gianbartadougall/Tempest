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

void tempest_update_system_state(void);
void tempest_update_motor_state(void);
void tempest_update_mode(void);
void tempest_update_mode_indicator(void);
void tempest_hardware_init(void);

void tempest_print_system_state(void);
void tempest_print_motor_state(void);

#endif // TEMPEST_H

