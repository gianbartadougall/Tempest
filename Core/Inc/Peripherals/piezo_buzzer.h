#ifndef PIEZO_BUZZER_H
#define PIEZO_BUZZER_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

typedef struct PiezoSound {
    uint8_t numMaxPlays;
    uint8_t index;
    uint16_t frequencies[5];
    uint8_t dutyCycles[5]; 
} PiezoSound;

extern PiezoSound sound1;
extern PiezoSound sound2;

void piezo_buzzer_init(void);
void piezo_buzzer_isr(void);
void piezo_buzzer_play_sound(PiezoSound sound);

#endif // PIEZO_BUZER_H