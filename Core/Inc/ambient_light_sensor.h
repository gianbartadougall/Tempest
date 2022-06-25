#ifndef AMBIENT_LIGHT_SENSOR_H
#define AMBIENT_LIGHT_SENSOR_H

#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"

void ambient_light_sensor_init(void);
uint8_t ambient_light_sensor_read(void);

#endif // AMBIENT_LIGHT_SENSOR_H