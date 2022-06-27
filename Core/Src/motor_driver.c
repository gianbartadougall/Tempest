/**
 * @file motor_driver.h
 * @author Gian Barta-Dougall
 * @brief Peripheral driver for a generic motor driver
 * @version 0.1
 * @date 2022-06-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/* Public Includes */

/* Private Includes */
#include "motor_driver.h"
#include "debug_log.h"

/* STM32 Includes */

/* Private #defines */
#define PERCENT_TO_DUTY_CYCLE(timer, percent) (uint32_t) ((timer->ARR / 100) * (percent))

#define NUM_MOTORS 1

/* Struct Declarations */

typedef struct Motor_TypeDef {
    uint32_t pins[2];
    GPIO_TypeDef* ports[2];
} Motor_TypeDef;

/* Variable Declarations */

Motor_TypeDef Motors[NUM_MOTORS];

uint32_t mdPins[NUM_MOTORS][2] = {
    {1, 0} // Pins for Motor 0
};

GPIO_TypeDef* mdPorts[NUM_MOTORS][2] = {
    {GPIOB, GPIOB} // Ports for Motor 0
};

/* Function prototypes */
void motor_driver_reset_motor(uint8_t motor);
uint8_t motor_driver_motor_is_invalid(uint8_t motor);

/* Function prototypes */
uint8_t motor_driver_invalid_port(uint8_t port);
uint8_t motor_driver_port_not_in_use(Motor_TypeDef* Motor, uint8_t port);

void motor_driver_init(void) {

    /* Initialise motors */
    for (int i = 0; i < NUM_MOTORS; i++) {
        
        // Copy ports and pins
        Motors[i].pins[0]  = mdPins[i][0];
        Motors[i].pins[1]  = mdPins[i][1];
        Motors[i].ports[0] = mdPorts[i][0];
        Motors[i].ports[1] = mdPorts[i][1];

        // Initialise pins to generic output
        for (int j = 0; j < 2; j++) {
            Motors[i].ports[j]->MODER   &= ~(0x03 << (Motors[i].pins[j] * 2)); // Clear mode
            Motors[i].ports[j]->MODER   |= (0x01 << (Motors[i].pins[j] * 2)); // Set mode to output
            Motors[i].ports[j]->OSPEEDR &= ~(0x03 << (Motors[i].pins[j] * 2)); // Set pin to low speed
            Motors[i].ports[j]->PUPDR   &= ~(0x03 << (Motors[i].pins[j] * 2)); // Set pin to no pull up/down
            Motors[i].ports[j]->OTYPER  &= ~(0x01 << Motors[i].pins[j]); // Set pin to push-pull

            // Enable clock for the GPIO pin
            if (Motors[i].ports[j] == GPIOA) {
                __HAL_RCC_GPIOA_CLK_ENABLE();
            } else if (Motors[i].ports[j] == GPIOB) {
                __HAL_RCC_GPIOB_CLK_ENABLE();
            }
        }
    }
}

HAL_StatusTypeDef motor_driver_set_motor_state(uint8_t motor, uint8_t state) {

    if (motor_driver_motor_is_invalid(motor)) {
        return HAL_ERROR;
    }

    // Reset the motor
    motor_driver_reset_motor(motor);
    
    switch (state) {
        case MOTOR_DRIVER_STOP:
            // Do nothing as motor has already been reset
            break;
        case MOTOR_DRIVER_DIRECTION_1:
            Motors[motor].ports[0]->ODR |= (0x01 << Motors[motor].pins[0]);
            break;
        case MOTOR_DRIVER_DIRECTION_2:
            Motors[motor].ports[1]->ODR |= (0x01 << Motors[motor].pins[1]);
            break;
        default:
            return HAL_ERROR;
    }

    return HAL_OK;
}

void motor_driver_reset_motor(uint8_t motor) {
    
    if (motor_driver_motor_is_invalid(motor)) {
        return;
    }

    // Reset pins
    Motors[motor].ports[0]->ODR &= ~(0x01 << Motors[motor].pins[0]);
    Motors[motor].ports[1]->ODR &= ~(0x01 << Motors[motor].pins[1]);
}

uint8_t motor_driver_motor_is_invalid(uint8_t motor) {
    return motor > (NUM_MOTORS - 1);
}

uint8_t motor_driver_get_motor_state(uint8_t motor) {

    if (motor_driver_motor_is_invalid(motor)) {
        return 255;
    }
    
    uint8_t pin0Index = 0x01 << Motors[motor].pins[0];
    uint8_t pin1Index = 0x01 << Motors[motor].pins[1];
    uint8_t pin0 = ((Motors[motor].ports[0]->ODR & pin0Index) == pin0Index ? GPIO_PIN_SET : GPIO_PIN_RESET);
    uint8_t pin1 = ((Motors[motor].ports[1]->ODR & pin1Index) == pin1Index ? GPIO_PIN_SET : GPIO_PIN_RESET);

    if (!pin0 && !pin1) {
        return MOTOR_DRIVER_STOP;
    }

    if (pin0 && !pin1) {
        return MOTOR_DRIVER_DIRECTION_1;
    }

    if (!pin0 && pin1) {
        return MOTOR_DRIVER_DIRECTION_2;
    }

    return 255;

}