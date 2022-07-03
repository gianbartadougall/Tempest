#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#define PUSH_BUTTON_0 0
#define PUSH_BUTTON_1 1

void pb_init(void);
void pb_0_isr(void);
void pb_1_isr(void);

void pb_start_timer(void);
void pb_stop_timer(void);
uint8_t pb_get_state(uint8_t pushButton);

uint8_t pb0_triggered_early(void);
uint8_t pb1_triggered_early(void);
void pb0_reset_timer(void);
#endif // PUSHBUTTON_H