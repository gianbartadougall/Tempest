/**
 * @file blind.h
 * @author Gian Barta-Dougall
 * @brief System file for blind
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef BLIND_H
#define BLIND_H

#include "stm32l4xx_hal.h"
#include "task_scheduler_1.h"

enum Mode { MANUAL, DAY_LIGHT, CONFIGURE_SETINGS };

#define NUM_BLINDS 2

#define BLIND_ID_OFFSET 52
#define BLIND_1_ID      (0 + BLIND_ID_OFFSET)
#define BLIND_2_ID      (1 + BLIND_ID_OFFSET)

typedef struct Blind {
    const uint8_t id;
    const uint8_t motorId;
    const uint8_t encoderId;
    const uint8_t alSensorId;
    const uint8_t ledId;
    const Task1 switchToDayLightModeTask;
    Task1* confirmEncoderInOperation;
    Task1* blinkLedTask;
    Task1* configSettingsSoundTask;
    uint8_t mode;
    uint8_t previousMode;
    uint8_t status;
    uint32_t lastEncoderCount;
} Blind;

/* Public Variable Declarations */

/**
 * @brief Initialise the system library.
 */
void blind_selector_init(void);

uint8_t blind_get_motor_id(uint8_t blindId);
uint8_t blind_get_encoder_id(uint8_t blindId);
void blind_cancel_encoder_checking_task(uint8_t blindId);
void blind_update_connection_status(void);
Blind* blind_get_blind_in_focus(void);

#endif // BLIND_H
