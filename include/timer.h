#ifndef PETERSON_H
#define PETERSON_H

#include <stdint.h>

#include "resets.h"

#define TICKS_BASE         0x40108000u
#define TICKS_TIMER0_CTRL   (*(volatile uint32_t *)(TICKS_BASE + 0x18))
#define TICKS_TIMER0_CYCLES (*(volatile uint32_t *)(TICKS_BASE + 0x1c))

#define TIMER0_BASE        0x400b0000u
#define TIMER0_TIMERAWH    (*(volatile uint32_t *)(TIMER0_BASE + 0x24))
#define TIMER0_TIMERAWL    (*(volatile uint32_t *)(TIMER0_BASE + 0x28))

void timer_init(void);

uint64_t timer_now_us(void);

#endif /* PETERSON_H */