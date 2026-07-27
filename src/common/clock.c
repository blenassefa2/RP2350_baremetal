#include "clock.h"
#include "xosc.h"
#include "pll.h"
#include "resets.h"
#include "debug.h"



void clk_init(void) {
    
    // temporarily change the system clock to source from ROSC
    *(volatile uint32_t *)(CLOCKS_CLK_SYS_CTRL) &= ~1u;                  
    while (!(*(volatile uint32_t *)(CLOCKS_CLK_SYS_SELECTED) & (1u << 0)));
    
   
    xosc_init();
  
    
    pll_init();
 
    *(volatile uint32_t *)(CLOCKS_CLK_SYS_CTRL) &= ~(0x7u << 5);   
    *(volatile uint32_t *)(CLOCKS_CLK_SYS_CTRL) |= 1u;             
    while (!(*(volatile uint32_t *)(CLOCKS_CLK_SYS_SELECTED) & (1u << 1)));
  
    // clk_peri running from clk_sys
    *(volatile uint32_t *)(CLOCKS_CLK_PERI_CTRL) = (0u << 5) | (1u << 11);  
  }

  uint32_t get_clk_sys(void){
    return CLK_PERI_HZ;
  }