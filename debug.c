#include "debug.h"
#include <stdint.h>

void debug_delay(void)
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

void semihost_exit()
{
    register uint32_t op asm("r0") = 0x18u;
    register uint32_t reason asm("r1") = 0x20026u;

    asm volatile (
        "bkpt 0xab\n"
        :
        : "r" (op), "r" (reason)
        : "memory"
    );

    for (;;) { }
}

uint64_t semihost_write_byte(const char *buf, uint32_t length) {
    // must be volatile or the buffer gets put in registers & optimized away
    volatile struct {
        // https://developer.arm.com/documentation/dui0375/g/What-is-Semihosting-/SYS-WRITE--0x05-
        // arguments, in order:
        // word 0 = file handle (1 = stdout)
        // word 1 = pointer to buffer
        // word 2 = length of buffer
        uint32_t fd;
        const char *buf;
        uint32_t len;
    } args;

    args.fd = 1;  // 1 = stdout
    args.buf = buf;
    args.len = length;

    asm (
         // r1 must contain a pointer to the arguments
         "movs r1, %[args]\n"
         // semihosting call number 0x05 = SYS_WRITE
         "movs r0, #5\n"
         // make the semihosting call: https://developer.arm.com/documentation/dui0375/g/What-is-Semihosting-/The-semihosting-interface
         "bkpt 0xab\n"
         :
         : [args] "r" (&args)
         : "r0", "r1", "cc", "memory"
    );

    uint64_t result = -1;

    asm volatile (

        // r0 must contain the result = 0
        // if r0 contains a number greater than 0
        // it indicates that number of bytes were failed to be written
        "movs %[result], r0\n"
        : [result] "=r" (result)
        :
        : "cc", "memory"
    );
   return result;
}
