#include "../include/shared.h"
#include <stdatomic.h>
#include <stdint.h>
#include <stdbool.h>

uint32_t shared_counter = 0;

uint32_t cpu0_counter = 0;
uint32_t cpu1_counter = 0;

volatile atomic_bool cpu1_running = false;
volatile atomic_bool cpu0_running = false;

volatile atomic_bool cache_flushed = false;
