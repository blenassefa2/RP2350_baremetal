#include "../include/timer.h"

void timer_init(void)
{
  // 1. unreset TIMER0
  *(volatile uint32_t *)(RESETS_RESET) &= ~RESET_TIMER0;
  while (!(*(volatile uint32_t *)(RESETS_RESET_DONE) & RESET_TIMER0));

  // 2. 1 tick every 12 cycles of the 12 MHz crystal = 1 us
  *(volatile uint32_t *)(TICKS_TIMER0_CYCLES) = 12;
  *(volatile uint32_t *)(TICKS_TIMER0_CTRL)   = 1;      // ENABLE
}

uint64_t timer_now_us(void)
{
  // same hi/lo/hi trick as the AON procedure you quoted
  uint32_t hi, lo;
  do {
    hi = TIMER0_TIMERAWH;
    lo = TIMER0_TIMERAWL;
  } while (hi != TIMER0_TIMERAWH);
  return ((uint64_t)hi << 32) | lo;
}
