/**
 * @file synchronous_interrupts.c
 * @author Gian Barta-Dougall
 * @brief System file for synchronous_interrupts
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "synchronous_interrupts.h"
#include "ambient_light_sensor.h"
#include "log.h"
#include "version_config.h"
#include "utilities.h"
#include "blind.h"
#include "hardware_config.h"

/* Private STM Includes */

/* Private #defines */
#define MS_TO_COUNT(ms) (ms / 10)

/* Private Structures and Enumerations */

/* Private Variable Declarations */
uint16_t syncTimer10US = 0;
uint16_t delayCount    = 0;
uint16_t syncTimerMS   = 0;
uint16_t syncTimerS    = 0;
uint16_t syncTimerM    = 0;
uint16_t syncTimerH    = 0;

extern uint32_t ambientLightSensorFlag;
extern uint32_t blindTasksFlag;

/* Private Function Prototypes */
void sync_timer_process_ms_flags(void);
void sync_timer_process_s_flags(void);
void sync_timer_process_m_flags(void);
void sync_timer_process_h_flags(void);

/* Public Functions */

/**
 * @brief Global interrupt for timer 6. Note that timer 6 is a 10ms
 * timer and will call an interrupt every 10ms
 *
 */
void TIM6_DAC_IRQHandler(void) {

    if ((TIM6->SR & TIM_SR_UIF) != 0) {

        // Clear the interrupt flag

        TIM6->SR = 0x00;

        syncTimer10US++;
        delayCount++;

        // Increment 10us timer
        if (syncTimer10US == 100) {
            syncTimer10US = 0;
            syncTimerMS++;
            sync_timer_process_ms_flags();
        } else {
            return;
        }

        if (syncTimerMS == 1000) {
            syncTimerMS = 0;
            syncTimerS++;
            sync_timer_process_s_flags();
        } else {
            return;
        }

        if (syncTimerS == 60) {
            syncTimerS = 0;
            syncTimerM++;
            sync_timer_process_m_flags();
        } else {
            return;
        }

        if (syncTimerM == 60) {
            syncTimerM = 0;
            syncTimerH++;
            sync_timer_process_h_flags();
        } else {
            return;
        }
    }
}

/* Private Functions */

void sync_timer_process_ms_flags(void) {}

void sync_timer_process_s_flags(void) {

    switch (syncTimerS) {
        case 1:
#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED
            FLAG_SET(ambientLightSensorFlag, ALS1_DISCHARGE_CAPACITOR);
            FLAG_SET(ambientLightSensorFlag, ALS2_DISCHARGE_CAPACITOR);
            // log_prints("Discharging\r\n");
#endif
            break;
        case 2:
#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED
            FLAG_SET(ambientLightSensorFlag, ALS1_RECORD_AMBIENT_LIGHT);
            FLAG_SET(ambientLightSensorFlag, ALS2_RECORD_AMBIENT_LIGHT);
            // log_prints("Recording ambient light\r\n");
#endif
            break;
        case 5:
#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED
            FLAG_SET(ambientLightSensorFlag, ALS1_READ_AMBIENT_LIGHT);
            FLAG_SET(ambientLightSensorFlag, ALS2_READ_AMBIENT_LIGHT);
            // log_prints("Reading ambient light\r\n");
#endif
            break;
        case 8:
#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED
            FLAG_SET(ambientLightSensorFlag, ALS1_CHARGE_CAPACITOR);
            FLAG_SET(ambientLightSensorFlag, ALS2_CHARGE_CAPACITOR);
            // log_prints("Charging Capacitor\r\n");
#endif
            break;
        case 9:
#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED
            FLAG_SET(ambientLightSensorFlag, ALS1_CONFIRM_CONNECTION);
            FLAG_SET(ambientLightSensorFlag, ALS2_CONFIRM_CONNECTION);
            // log_prints("Confirming connection\r\n");
#endif
            break;
        case 10:
#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED
            log_prints("day light udpate!\r\n");
            FLAG_SET(blindTasksFlag, FUNC_ID_BLINDS_DAY_LIGHT_UPDATE);
#endif
            break;
        default:
            break;
    }
}

void sync_timer_delay10us(uint16_t delay10us) {
    log_prints("1\r\n");
    delayCount = 0;

    while (delayCount < delay10us) {
        // char m[60];
        // sprintf(m, "%i\r\n", delayCount);
        // log_prints(m);
    }
    log_prints("exiting\r\n");
    // uint32_t endTime = ((syncTimer10US + delay10us) % 100);

    // if (endTime < syncTimer10US) {

    //     while (syncTimer10US > endTime) {
    //         log_prints("2\r\n");
    //     }
    // }

    // while (syncTimer10US < endTime) {
    //     char m[60];
    //     sprintf(m, "T: %i\r\n", syncTimer10US);
    //     log_prints(m);
    // }
}

void sync_timer_process_m_flags(void) {}

void sync_timer_process_h_flags(void) {}