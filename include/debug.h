#ifndef DEBUG_H
#define DEBUG_H

#define GPIO_FUNCSEL_LED 5

#include <stdint.h>

#define LED_PIN 19

void debug_blink(int);
void configure_led();
uint64_t semihost_write_byte(const char *buf, uint32_t length);
#endif