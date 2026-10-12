
#include <stdint.h>

#include "../include/dwt.h"

#define DEMCR       0xE000EDFCu
#define DWT_CTRL    0xE0001000u
#define DWT_CYCCNT  0xE0001004u

void cycle_init(void)
{
    *(volatile uint32_t *)DEMCR      |= (1u << 24);
    *(volatile uint32_t *)DWT_CYCCNT  = 0;
    *(volatile uint32_t *)DWT_CTRL   |= 1u;
}

uint32_t cycle_now(void)
{
  return *(volatile uint32_t *)(DWT_CYCCNT);
}