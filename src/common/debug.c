#include "debug.h"
#include "gpio.h"
// #include "resets.h"

void debug_delay(void)
{
    for (volatile uint32_t i = 0; i < 300000u; i++) { }
}

void configure_led()
{
    // function 5 = SIO
    *(volatile uint32_t *)(IO_BANK0_GPIO19_CTRL) = GPIO_FUNCSEL_LED;
    *(volatile uint32_t *)(PADS_BANK0_GPIO19) = 0x34;
    // enable output
    *(volatile uint32_t *)(SIO_GPIO_OE_SET) = 0x01U << LED_PIN;
}
void turn_led_on()
{
    *(volatile uint32_t *)(SIO_GPIO_OUT_SET) = 0x01U << LED_PIN;
}

void turn_led_off()
{
    *(volatile uint32_t *)(SIO_GPIO_OUT_CLR) = 0x01U << LED_PIN;
}

void debug_blink(int times)
{
    for (int i = 0; i < times; i++)
    {
        turn_led_on();
        debug_delay();
        turn_led_off();
        debug_delay();
    }
    debug_delay();
    debug_delay();  // extra pause so blink groups are distinguishable
}

static void semihost_put_hex(uint8_t byte)
{
    static const char hex_digits[] = "0123456789abcdef";
    char out[3];

    out[0] = hex_digits[(byte >> 4) & 0x0F];
    out[1] = hex_digits[byte & 0x0F];
    out[2] = '\0';

    semihost_write_byte(out, 1);
}

static void semihost_puts(char *s)
{
    // Compute length manually — avoid depending on <string.h>/strlen
    // if you don't have a libc linked in.
    uint32_t len = 0;
    while (s[len] != '\0')
    {
        len++;
    }

    if (len == 0)
    {
        return;
    }

    semihost_write_byte(s, len);
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
        // Based on arm documentation:
        //      r0 must contain the result = 0
        //      if r0 contains a number greater than 0
        //      it indicates that number of bytes were failed to be written
        "movs %[result], r0\n"
        : [result] "=r" (result)
        :
        : "cc", "memory"
    );
   return result;
}
