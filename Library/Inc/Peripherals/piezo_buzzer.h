#ifndef PIEZO_BUZZER_H
#define PIEZO_BUZZER_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

// The index of these sounds are important as the index
// defines the type of sound played
#define SOUND_LENGTH  21
#define SOUND         (0 * SOUND_LENGTH)
#define SOUND1        (1 * SOUND_LENGTH)
#define SUCCESS_SOUND (2 * SOUND_LENGTH)
#define ERROR_SOUND   (3 * SOUND_LENGTH)

enum PiezoSoundModes { UNLIMITED, ONE_TIME, TWO_TIMES, THREE_TIMES, FOUR_TIMES, FIVE_TIMES };

typedef struct PiezoNote {
    const uint16_t frequency;
    const uint8_t dutyCycle;
} PiezoNote;

void piezo_buzzer_init(void);
void piezo_buzzer_isr(void);
void piezo_buzzer_play_sound(uint8_t sound);

#endif // PIEZO_BUZER_H