#ifndef AMBIENT_LIGHT_SENSOR_H
#define AMBIENT_LIGHT_SENSOR_H

#include "stm32l432xx.h"
#include "stm32l4xx_hal.h"

/* Public Enums */
enum AmbientLightLevel {HIGH, LOW, UNDETERMINED};

void ambient_light_sensor_init(void);
void ambient_light_sensor_isr_s1(void);
void ambient_light_sensor_isr_s2(void);
void ambient_light_sensor_isr_s3(void);
enum AmbientLightLevel ambient_light_read(void);

#endif // AMBIENT_LIGHT_SENSOR_H