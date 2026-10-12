#ifndef SHARED_H
#define SHARED_H


#include <stdint.h>
#include <stdatomic.h>
#include <stdbool.h>


#define ITERATIONS 4100



extern uint32_t shared_counter;
extern uint32_t cpu0_counter;
extern uint32_t cpu1_counter;



extern volatile atomic_bool cpu1_running;
extern volatile atomic_bool cpu0_running;

extern volatile atomic_bool cache_flushed;



#endif