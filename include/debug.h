#ifndef DEBUG_H
#define DEBUG_H



#include <stdint.h>

#define LED_PIN 19

void debug_blink(int);
void configure_led();
void turn_led_on();
void turn_led_off();
uint64_t semihost_write_byte(const char *buf, uint32_t length);
#endif