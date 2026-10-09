// core0.c
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdbool.h>
#include <stdint.h>
#include "debug.h"


// Exception handlers (shared by both cores via the vector table)
void Default_Handler(void);
void Reset_Handler(void);
void NMI_Handler(void)          __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler(void)    __attribute__ ((weak));
void MemManage_Handler(void)    __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler(void)     __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler(void)   __attribute__ ((weak, alias("Default_Handler")));
void SecureFault_Handler(void)  __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler(void)          __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler(void)     __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler(void)       __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler(void)      __attribute__ ((weak, alias("Default_Handler")));

void Default_Handler(void)
{
  while (1);
}

void HardFault_Handler(void)
{
  configure_led();
  debug_blink(8);   // distinct pattern = "I crashed"
  while (1);
}


// Core 1 launch (RP2350 SIO FIFO handshake)
#define SIO_BASE     0xd0000000u
#define SIO_FIFO_ST  (*(volatile uint32_t *)(SIO_BASE + 0x50))
#define SIO_FIFO_WR  (*(volatile uint32_t *)(SIO_BASE + 0x54))
#define SIO_FIFO_RD  (*(volatile uint32_t *)(SIO_BASE + 0x58))
#define FIFO_ST_VLD  (1u << 0)   // read FIFO has data
#define FIFO_ST_RDY  (1u << 1)   // write FIFO has space

extern uint32_t __Vectors[];     // from minimum_arm_image_def_block.s
extern void core1_main(void);    // in core1.c

// Core 1 stack (core 0 uses the stack defined in the linker script)
#define CORE1_STACK_WORDS 1024
static uint32_t core1_stack[CORE1_STACK_WORDS] __attribute__((aligned(8)));

static void fifo_drain(void)
{
  while (SIO_FIFO_ST & FIFO_ST_VLD) {
    (void)SIO_FIFO_RD;
  }
}

static void fifo_push(uint32_t v)
{
  while (!(SIO_FIFO_ST & FIFO_ST_RDY));
  SIO_FIFO_WR = v;
  __asm volatile ("sev");
}

static uint32_t fifo_pop(void)
{
  while (!(SIO_FIFO_ST & FIFO_ST_VLD)) {
    __asm volatile ("wfe");
  }
  return SIO_FIFO_RD;
}

static void launch_core1(void (*entry)(void))
{
  uint32_t cmd[] = {
    0, 0, 1,
    (uint32_t)__Vectors,                          // VTOR for core 1
    (uint32_t)&core1_stack[CORE1_STACK_WORDS],    // initial SP (top of stack)
    (uint32_t)entry                               // entry (thumb bit already set)
  };

  uint32_t i = 0;
  while (i < sizeof(cmd) / sizeof(cmd[0])) {
    uint32_t c = cmd[i];
    if (c == 0) {
      fifo_drain();
      __asm volatile ("sev");
    }
    fifo_push(c);
    uint32_t r = fifo_pop();
    i = (r == c) ? i + 1 : 0;   // on mismatch, restart the sequence
  }
}


void core0_main(void)
{
  configure_led();      // done once here; core 1 reuses the same setup

  debug_blink(2);       // core 0: blink twice

  launch_core1(core1_main);

  while (1);            // park
}