/**
 * @file seriam_comms.h
 * @author Gian Barta-Dougall
 * @brief Library for communicating with mcu over UART
 * @version 0.1
 * @date 2022-05-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SERIAL_COMMS_H
#define SERIAL_COMMS_H

/* Public Includes */
#include <stdint.h>
#include <stdio.h>

/* STM32 Includes */

void serial_comms_clear_buffer(void);

void serial_comms_add_to_buffer(char c);

#endif // SERIAL_COMMS_H