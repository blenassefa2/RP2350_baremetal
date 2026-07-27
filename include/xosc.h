#ifndef XOSC_H
#define XOSC_H
#include <stdint.h>

#define XOSC_CTRL_FREQ_RANGE_VALUE_1_15MHZ 0xaa0

#define XOSC_HZ 12000000u
#define STARTUP_DELAY (((XOSC_HZ / 1000) + 128) / 256)   // ~47

#define XOSC_CTRL_ENABLE_VALUE_ENABLE 0xfab
#define XOSC_CTRL_ENABLE_LSB    12

#define XOSC_STATUS_STABLE_BITS   0x80000000

#define XOSC_BASE 0x40048000
#define XOSC_CTRL   (XOSC_BASE + 0x0)
#define XOSC_STATUS (XOSC_BASE + 0x4)
#define XOSC_STARTUP (XOSC_BASE + 0xc)


void xosc_init();
#endif

