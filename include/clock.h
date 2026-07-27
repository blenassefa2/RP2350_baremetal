#ifndef CLOCK_H
#define CLOCK_H


#include <stdint.h>

#define CLOCKS_BASE 0x40010000
#define CLOCKS_CLK_SYS_CTRL (CLOCKS_BASE + 0x3c)
#define CLOCKS_CLK_SYS_SELECTED (CLOCKS_BASE + 0x44)

#define CLOCKS_CLK_PERI_CTRL (CLOCKS_BASE + 0x48)

#define CLK_PERI_HZ 150000000u 

void clk_init(void);
uint32_t  get_clk_sys(void); 

#endif