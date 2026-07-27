#include "pll.h"
#include "resets.h"
#include "debug.h"

void pll_init()
{

    //Bring PLL_SYS out of reset
    *(volatile uint32_t *)(RESETS_RESET + REG_ALIAS_SET_BITS) = RESETS_PLL_BIT;
    *(volatile uint32_t *)(RESETS_RESET + REG_ALIAS_CLR_BITS) = RESETS_PLL_BIT;
    while (!(*(volatile uint32_t *)(RESETS_RESET_DONE) & RESETS_PLL_BIT));
  

    // Configure PLL_SYS for 150 MHz
    *(volatile uint32_t *)(PLL_SYS_CS) = 1;      //PLL_SYS_REFDIV          
    *(volatile uint32_t *)(PLL_SYS_FBDIV_INT) = 125;      // XOSC_HZ / PLL_SYS_VCO_FREQ_HZ(1500 MHz)

    
    *(volatile uint32_t *)(PLL_SYS_PWR) &= ~(PLL_PWR_PD_BITS | PLL_PWR_VCOPD_BITS);  
    while (!( *(volatile uint32_t *)(PLL_SYS_CS) & PLL_CS_LOCK_BITS));               
    
  
    *(volatile uint32_t *)(PLL_SYS_PRIM) = (5u << PLL_PRIM_POSTDIV1_LSB) | (2u << PLL_PRIM_POSTDIV2_LSB);
    *(volatile uint32_t *)(PLL_SYS_PWR) &= ~PLL_PWR_POSTDIVPD_BITS;                
  
}