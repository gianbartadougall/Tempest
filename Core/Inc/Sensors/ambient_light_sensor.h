#ifndef AMBIENT_LIGHT_SENSOR_H
#define AMBIENT_LIGHT_SENSOR_H

#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"

/* Public Enums */

#define AL_SENSOR_OFFSET 43
#define AL_SENSOR_1      (0 + AL_SENSOR_OFFSET)
#define AL_SENSOR_2      (1 + AL_SENSOR_OFFSET)

void als_mode_input(uint8_t alsId);
void als_mode_reset(uint8_t alsId);
void als_update_status(uint8_t alsId);
uint8_t als_light_found(uint8_t alsId);

#endif // AMBIENT_LIGHT_SENSOR_H