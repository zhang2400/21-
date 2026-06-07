#ifndef __MUSCI_H_
#define __MUSCI_H_
#include "headfile.h"

// 音符结构体
typedef struct {
    uint32_t frequency;
    uint32_t duration;
} Note;

extern const Note qing_tian[243];

void play_music(const Note *song, uint32_t length);

#endif