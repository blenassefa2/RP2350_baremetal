#include "xosc.h"

void xosc_init()
{
    // Start the 12 MHz crystal
    *(volatile uint32_t *)(XOSC_CTRL) = XOSC_CTRL_FREQ_RANGE_VALUE_1_15MHZ;
    *(volatile uint32_t *)(XOSC_STARTUP) = STARTUP_DELAY;
    *(volatile uint32_t *)(XOSC_CTRL) |= (XOSC_CTRL_ENABLE_VALUE_ENABLE << XOSC_CTRL_ENABLE_LSB);
    while (!(*(volatile uint32_t *)(XOSC_STATUS) & XOSC_STATUS_STABLE_BITS ));

}