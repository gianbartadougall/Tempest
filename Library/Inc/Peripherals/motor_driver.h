#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

/* Defining motor ports */
#define MD_MOTOR_0 0
#define MD_MOTOR_1 1

/* Defining motor states */
#define MOTOR_DRIVER_STOP        0
#define MOTOR_DRIVER_DIRECTION_1 1
#define MOTOR_DRIVER_DIRECTION_2 2

/* Function prototypes */

/**
 * @brief Sets the GPIO pins and ports in the Motor type def to the given ports and pins
 * passed into the function.The
 *
 * @param MotorHandle Handle to the Motor_TypeDef
 * @param ports The ports of the pins used to control the output pins of the motor driver.
 * The ordering is important and should go {Port A1, Port A2, Port B1, Port B2}
 * @param pins The pins used to control the output pins of the motor driver. The ordering
 * is important and should go {Pin A1, Pin A2, Pin A3, Pin A4}
 */
void motor_driver_init(void);

/**
 * @brief Sets the state of a given motor
 *
 * @param motor The motor to update the state of
 * @param state The state the motor should be updated to
 * @return HAL_StatusTypeDef HAL_ERROR if the given motor was invalid or the
 *         given state was invalid
 */
HAL_StatusTypeDef motor_driver_set_motor_state(uint8_t motor, uint8_t state);

uint8_t motor_driver_get_motor_state(uint8_t motor);

uint8_t motor_driver_motor_is_off(uint8_t motor);

#endif // MOTOR_DRIVER_h