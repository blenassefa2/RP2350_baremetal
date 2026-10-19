// core1.c
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debug.h"
#include <stdatomic.h>
#include <stdint.h>
// #include "timer.h"
#include "shared.h"
#include "uart.h"
#include "dwt.h"

#define  WAIT_WINDOW 0
#define  SPY_TIME 1000

#define THRESHOLD 7000

__attribute__((noinline))
void flush_icache(void) {
    asm volatile(
        ".align 6\n"

        "1:\n"
        ".rept 8192\n"
        "nop\n"
        ".endr\n"

        :
        :
        : "memory"
    );
}
uint32_t foo_time[SPY_TIME];
uint32_t timestamp_core1[SPY_TIME];
uint32_t foo_count;

extern void foo();
#define BAUD_RATE 115200
uint32_t s, e = -1;
void core1_main(void)
{
    // initialize(BAUD_RATE);
    cycle_init();
  debug_blink(1);       // core 1: blink four times
 
  for (int i = 0; i < SPY_TIME; i++)
  {   
      foo_time[i] = 0;
  }
  atomic_store_explicit(&cpu1_running, true, memory_order_release);
 
  // int counter = 0;
  while(cpu1_counter < SPY_TIME && atomic_load_explicit(&cpu0_running, memory_order_acquire)){
      
      flush_icache();
      cpu1_counter++;

      while (cpu1_counter > cpu0_counter);

      s = cycle_now();
      foo();
      e = cycle_now();
      
      foo_time[cpu1_counter - 1] = (uint32_t)(e - s);
      timestamp_core1[cpu1_counter - 1] = e;
     
      
  
  }

  atomic_store_explicit(&cpu1_running, false, memory_order_release);
  
  while (1);  // park core 1
}
