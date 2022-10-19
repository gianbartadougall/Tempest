#ifndef PIEZO_BUZZER_H
#define PIEZO_BUZZER_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#define SOUND             0
#define PIEZO_ERROR_SOUND 42

enum PiezoSoundModes { UNLIMITED, ONE_TIME, TWO_TIMES, THREE_TIMES, FOUR_TIMES, FIVE_TIMES };

typedef struct PiezoNote {
    const uint16_t frequency;
    const uint8_t dutyCycle;
} PiezoNote;

void piezo_buzzer_init(void);
void piezo_buzzer_isr(void);
void piezo_buzzer_play_sound(uint8_t sound);

#endif // PIEZO_BUZER_H