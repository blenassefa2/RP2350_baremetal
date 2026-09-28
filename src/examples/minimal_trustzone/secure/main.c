#include <stdint.h>
#include "tz_intrinsics.h"

#define PADS_LED_REG   (*(volatile uint32_t*)0x4002809c)
#define IOBANK_LED_REG (*(volatile uint32_t*)0x40038050)
#define SIO_OUT_SET    (*(volatile uint32_t*)0xd0000018)
#define SIO_OUT_CLR    (*(volatile uint32_t*)0xd0000020)
#define SIO_OE_SET     (*(volatile uint32_t*)0xd0000038)
#define LED_MASK       0x80000u

static void led_init(void) {
    PADS_LED_REG = 0x5;
    IOBANK_LED_REG = 0x34;
    SIO_OE_SET = LED_MASK;
}
static void delay(volatile uint32_t n) { while (n--) __asm volatile("nop"); }
static void blink(int n) {
    for (int i = 0; i < n; i++) {
        SIO_OUT_SET = LED_MASK; delay(1000000);
        SIO_OUT_CLR = LED_MASK; delay(1000000);
    }
    delay(5000000);
}

#define SAU_CTRL (*(volatile uint32_t*)0xE000EDD0)
#define SAU_RNR  (*(volatile uint32_t*)0xE000EDD8)
#define SAU_RBAR (*(volatile uint32_t*)0xE000EDDC)
#define SAU_RLAR (*(volatile uint32_t*)0xE000EDE0)
#define SCB_SHCSR (*(volatile uint32_t*)0xE000ED24)
#define SCB_VTOR_NS (*(volatile uint32_t*)0xE002ED08)
#define SHCSR_SECUREFAULTENA (1u << 19)

static void sau_region(uint8_t n, uint32_t base, uint32_t limit_inclusive, int nsc) {
    SAU_RNR = n;
    SAU_RBAR = base & ~0x1Fu;
    SAU_RLAR = (limit_inclusive & ~0x1Fu) | (nsc ? (1u << 1) : 0) | 1u;
}

#define NSC_BASE   0x1003F000u
#define NSC_LIMIT  0x1003FFFFu
#define NS_RAM_BASE  0x20040000u
#define NS_RAM_LIMIT 0x2007FFFFu
#define NS_FLASH_BLOB 0x10100000u
#define NS_IMAGE_COPY_BYTES 0x8000u  /* 32KB */

/* result mailbox, Secure RAM, fixed section so it's easy to find over SWD */
volatile uint32_t s_result[4] __attribute__((used, section(".s_result")));
enum { SR_MARK = 0, SR_SFSR, SR_NS_MARKER, SR_SPARE };

void HardFault_Handler(void) {
    s_result[SR_MARK] = 0xBADBAD00u;
    blink(1);       /* unexpected escalation -- SECUREFAULTENA didn't work */
    while (1) { }
}

void SecureFault_Handler(void) {
    s_result[SR_MARK] = 0x5ECF0000u;
    s_result[SR_SFSR]  = *(volatile uint32_t*)0xE000EDE4u;
    blink(3);       /* expected outcome: illegal NS access correctly caught */
    while (1) { }
}

void Default_Handler(void) { while (1) { } }
void NMI_Handler(void)        __attribute__((alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((alias("Default_Handler")));

typedef void (*ns_entry_t)(void) __attribute__((cmse_nonsecure_call));
int main(void) {
    led_init();
    blink(1);   /* milestone 1: Secure booted */

    sau_region(0, NSC_BASE, NSC_LIMIT, 1);
    sau_region(1, NS_RAM_BASE, NS_RAM_LIMIT, 0);
    SAU_CTRL = 1;
    __asm volatile("dsb");
    __asm volatile("isb");
    blink(2);   /* milestone 2: SAU configured */

    SCB_SHCSR |= SHCSR_SECUREFAULTENA;
    blink(10);   /* milestone 3: SecureFault enabled */

    const uint32_t *src = (const uint32_t*)NS_FLASH_BLOB;
    uint32_t *dst = (uint32_t*)NS_RAM_BASE;
    for (uint32_t i = 0; i < NS_IMAGE_COPY_BYTES/4; i++) dst[i] = src[i];
    blink(4);   /* milestone 4: NS image copied */

    const uint32_t *ns_vt = (const uint32_t*)NS_RAM_BASE;
    SCB_VTOR_NS = NS_RAM_BASE;
    __TZ_set_MSP_NS(ns_vt[0]);

    ns_entry_t ns_entry = (ns_entry_t)(ns_vt[1] & ~1u);
    blink(5);   /* milestone 5: about to launch NS */
    ns_entry();

    blink(1);   /* should never happen: NS returned */
    while (1) { }
}