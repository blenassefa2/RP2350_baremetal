// core0.c
// SPDX-License-Identifier: GPL-3.0-or-later


#include <stdbool.h>
#include <stdint.h>
#include "debug.h"
#include "shared.h"
#include "flash.h"
#include "uart.h"
#include "dwt.h"

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


static uint32_t lcg_state = 0x12345678u;

static int next_random_byte(void)
{
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return (int)(lcg_state >> 16);
}

void sort_data(int* data)
{
    for (int i = 0; i < 1000; i++) {
        for (int j = i + 1; j < 1000; j++) {
            if (data[i] > data[j]) {
                int temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }
        }
    }
}
static int* get_random_data(void)
{
    static int data[1000] = {0};
    for (int i = 0; i < 1000; i++) {
        data[i] = next_random_byte();
    }

    sort_data(data);

    return data;
}




#define REPEATS 1000
#define SECRET 60

// __attribute__((noinline, section(".victim_text"), used))
// static uint32_t additional_block(uint32_t x) {
//     // Keep this instruction-heavy so accesses are dominated by XIP fetches.
//     x ^= 0xA5A5A5A5u;
//     x = (x << 3) | (x >> 29);
//     x += 0x1234u;
//     // __asm volatile ("nop\n nop\n nop\n nop");
//     return x;
// }

void not_important(){
    __asm volatile ("nop\n nop\n nop\n nop");
}
/**
* This is the function I want to check if it is executed or not executed
*/
__attribute__((section(".victim_text"))) void foo(void) {
    volatile uint32_t sink = 0;
    uint32_t x = 64;
    x ^= 0xA5A5A5A5u;
    x = (x << 3) | (x >> 29);
    x += 0x1234u;
      sink ^=(uint32_t)x;
    
    (void)sink;
}

extern uint32_t foo_time[4600];

/**

variable naming:
#-of-times_cache-state_iteration_time


cache states cold, flushed and disabled should be equivalent/somewhat similar

cache state warm should be significantly lower than all the three above

 */

uint32_t empty_iterations_time = -1;

uint32_t single_cold_iteration_time = -1;
uint32_t single_warm_iteration_time = -1;

uint32_t single_flushed_iteration_time = -1;

uint32_t single_disabled_iteration_time = - 1;
uint32_t timestamp_core0[REPEATS];
uint32_t zero[REPEATS];



void core0_main(void) {
    // systick_init();
    int *secret_data = get_random_data();
    while (!atomic_load_explicit(&cpu1_running, memory_order_acquire));
    int counter  = 0;
    int d = 0;
    while (cpu0_counter < REPEATS){
       
        while (!atomic_load_explicit(&cpu0_continue, memory_order_acquire));
       
        if (counter == 0)
        {
          uint32_t ti = cycle_now();
            foo();
            foo();
            uint32_t tf = cycle_now();
            zero[cpu0_counter] = tf - ti;
            d++;
        }
        else
        {
            // not_important();
        }

        counter++;
        
        timestamp_core0[cpu0_counter] = cycle_now();
        cpu0_counter++;
        // this method is constant and always have a constant distance before executing foo
        if (counter == SECRET) {

            counter = 0;

        }
        shared_counter += 1;

        atomic_store_explicit(&cpu1_probe, true, memory_order_release);
        atomic_store_explicit(&cpu0_continue, false, memory_order_release);

    }
    atomic_store_explicit(&cpu0_running, false, memory_order_release);
    
 
    while(1); // park core 0
}
#define BAUD_RATE 115200
int main(void) {

  initialize(BAUD_RATE);
    configure_led();      // done once here; core 1 reuses the same setup
    
    cycle_init();
    
    debug_blink(1);       // core 0: blink twice
    uint32_t stack_top = (uint32_t)(core1_stack + 256);
    atomic_store_explicit(&cpu0_running, true, memory_order_release);
    launch_core1(core1_main);
    enable_xip_cache(true); 
    uart_puts("Core 0 started\n");
    core0_main();
    uart_puts("Core 0 finished\n");
    while (1);
}