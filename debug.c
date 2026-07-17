#include "debug.h"

static void debug_delay(void)
{
    for (volatile uint32_t i = 0; i < 3000000u; i++) { }
}

void debug_blink(int times)
{
    for (int i = 0; i < times; i++)
    {
        SIO->GPIO_OUT_SET = 0x01U << DEBUG_LED_PIN;
        debug_delay();
        SIO->GPIO_OUT_CLR = 0x01U << DEBUG_LED_PIN;
        debug_delay();
    }
    debug_delay();
    debug_delay();  // extra pause so blink groups are distinguishable
}
