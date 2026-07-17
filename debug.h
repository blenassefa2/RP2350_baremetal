#ifndef DEBUG_H
#define DEBUG_H

#include "RP2350.h"
#include "core_cm33.h"


#include <stdint.h>

#define DEBUG_LED_PIN 19

void debug_blink(int);
uint64_t semihost_write_byte(const char *buf, uint32_t length);
#endif