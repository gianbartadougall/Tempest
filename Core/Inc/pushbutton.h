#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#define PUSH_BUTTON_0 0
#define PUSH_BUTTON_1 1
#define PUSH_BUTTON_2 2
#define PUSH_BUTTON_3 3

void pb_init(void);
void pb_0_isr(void);
void pb_1_isr(void);
uint8_t pb_get_state(uint8_t pushButton);

#endif // PUSHBUTTON_H