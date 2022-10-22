#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#define PUSH_BUTTON_0 0
#define PUSH_BUTTON_1 1

/* Private Includes */
#include "timer_ms.h"

/* Public Variables */

// Modes the pushbuttons can assume. These modes dictate which
// function pointer is called when an ISR is triggered. Picking
// larger numbers to keep them unique among other #defines
#define PB_MODE_DEFAULT 96
#define PB_MODE_0       97
#define PB_MODE_1       98
#define PB_MODE_2       99

/* Public Function Prototypes */

/**
 * @brief Initiailises the GPIO pins to interface with the
 * pushbuttons
 */
void pb_init(void);

/**
 * @brief Function called by interrupts triggered by rising/falling
 * edges of any pushbutton
 *
 * @param x The index of the pushbutton. The ISR uses this to determine
 * which pushbutton triggered the interrupt
 */
void pb_isr(uint8_t x);

/**
 * @brief Returns the state of a given pushbutton (either 1 or 0)
 * depending on the voltage at the GPIO pin of the connected pushbutton
 *
 * @param pushButton The index of the pushbutton to query
 * @return uint8_t The state of the pushbutton (1 or 0)
 */
uint8_t pb_get_state(uint8_t pushButton);

/**
 * @brief Sets the mode of all pushbuttons to the given mode. The
 * mode dictates which ISR pointer is called in the ISR function
 *
 * @param mode The mode to set the pushbuttons to
 */
void pb_set_mode(uint8_t mode);

/**
 * @brief Function used to setup the function pointers for each
 * pushbutton.
 *
 * @param x The index of the pushbutton being set up
 * @param mode The mode these callbacks should be used for
 * @param re_callback The function that the ISR will call when the
 * pushbuttons are in the given mode and a rising edge is triggered
 * @param fe_callback The function that the ISR will call when the
 * pushbuttons are in the given mode and a falling edge is triggered
 */
void pb_set_callbacks(uint8_t x, uint8_t mode, void (*re_callback)(void), void (*fe_callback)(void));

#endif // PUSHBUTTON_H