#ifndef PLL_H
#define PLL_H

#include <stdint.h>

#define PLL_SYS_BASE 0x40050000

#define PLL_SYS_CS          (PLL_SYS_BASE + 0x0)
#define PLL_SYS_FBDIV_INT    (PLL_SYS_BASE + 0x08)
#define PLL_SYS_PWR         (PLL_SYS_BASE + 0x04)
#define PLL_SYS_PRIM        (PLL_SYS_BASE + 0x0c)


#define PLL_PWR_PD_BITS         (1u << 0)
#define PLL_PWR_VCOPD_BITS      (1u << 5)
#define PLL_CS_LOCK_BITS        (1u << 31)
#define PLL_PWR_POSTDIVPD_BITS  (1u << 3)

#define PLL_PRIM_POSTDIV1_LSB     16u
#define PLL_PRIM_POSTDIV2_LSB     12u

void pll_init();
#endif