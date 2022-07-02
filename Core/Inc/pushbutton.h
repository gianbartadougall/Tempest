#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#define PUSH_BUTTON_0 0
#define PUSH_BUTTON_1 1

void pb_init(void);
void pb_0_isr_edge_detect(void);
void pb_1_isr_edge_detect(void);

void pb_start_timer(void);
void pb_stop_timer(void);
uint8_t pb_get_state(uint8_t pushButton);

#endif // PUSHBUTTON_H