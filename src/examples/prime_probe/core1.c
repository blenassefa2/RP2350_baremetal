//core1.c
#include "debug.h"
#include <stdatomic.h>
#include <stdint.h>
#include "shared.h"
#include "uart.h"
#include "dwt.h"

#define WAIT_WINDOW  0        // optional extra spin-cycles between prime and probe
#define SPY_TIME     1000

 #define  FOO_SETS 8


 #define THRESHOLD 7000
 
 extern void spy0(void);
 extern void spy1(void);
 extern void spy2(void);
 extern void spy3(void);
 extern void spy4(void);
 extern void spy5(void);
 extern void spy6(void);
 extern void spy7(void);

static void (*addrs[FOO_SETS])(void) = {
    spy0, spy1, spy2, spy3,
    spy4, spy5, spy6, spy7
};
static inline void prime(void)
{
    for (int i = 0; i < FOO_SETS; i++) {
        ((void (*)(void))(addrs[i]))();
    }
}

static inline void probe(void)
{
    for (int i = 0; i < FOO_SETS; i++) {
        ((void (*)(void))(addrs[i]))();
    }
}


uint32_t foo_time[SPY_TIME];
uint32_t foo_time2[SPY_TIME];
uint32_t timestamp_core1[SPY_TIME];
uint32_t foo_count;

void core1_main(void)
{
    cycle_init();
    debug_blink(1);       // core 1: blink four times

    for (int i = 0; i < SPY_TIME; i++) {
        foo_time[i] = 0;
    }

    atomic_store_explicit(&cpu1_running, true, memory_order_release);

    while (cpu1_counter < SPY_TIME && atomic_load_explicit(&cpu0_running, memory_order_acquire)) {

        prime();                  // execute all 7 sets' eviction sets
        
        probe();
        probe();
        
        cpu1_counter++;
        
        atomic_store_explicit(&cpu0_continue, true, memory_order_release);
        while (!atomic_load_explicit(&cpu1_probe, memory_order_acquire));   // let core0 run (may call foo here)

        
       
        uint32_t t0 = cycle_now();
        probe();
        uint32_t t1 = cycle_now();
        foo_time[cpu1_counter - 1] = t1 - t0;

        timestamp_core1[cpu1_counter - 1] = t1;

        atomic_store_explicit(&cpu1_probe, false, memory_order_release);
    }

    atomic_store_explicit(&cpu1_running, false, memory_order_release);

    while (1);  // park core 1
}