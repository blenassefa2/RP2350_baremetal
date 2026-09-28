#include <stdint.h>

extern int func1(int);
extern int func2(int (*)(int), int);

static int func3(int x) { return x + 4; }

volatile uint32_t *ns_mailbox = (volatile uint32_t*)0x2007C000;

void main(void) {
    ns_mailbox[0] = 0x4E530001u;          /* NS alive */
    ns_mailbox[1] = func1(1);             /* expect 4 */
    ns_mailbox[2] = func2(func3, 2);      /* expect 9 */
    ns_mailbox[3] = 0x4E530002u;          /* both veneer calls completed */

    /* deliberate illegal access -- should trigger SecureFault */
    volatile uint32_t leak = *(volatile uint32_t*)0x20000000u;
    ns_mailbox[0] = leak;  /* should never execute */

    while (1) { }
}