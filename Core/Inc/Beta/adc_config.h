/**
 * @file adc_config.h
 * @author Gian Barta-Dougall
 * @brief System file for adc_config
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef ADC_CONFIG_H
#define ADC_CONFIG_H

/* Public Includes */

/* Public STM Includes */
#include "stm32l4xx.h"

/* Public #defines */

/* Public Structures and Enumerations */

/* Public Variable Declarations */

/* Public Function Prototypes */

/**
 * @brief Initialise the system library.
 */
void adc_config_init(void);

void adc_config_als1_init(void);
void adc_config_disable_adc1(void);
uint16_t adc_config_adc1_convert(void);
#endif // ADC_CONFIG_H
