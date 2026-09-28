#ifndef TZ_INTRINSICS_H
#define TZ_INTRINSICS_H
#include <stdint.h>

static inline void __TZ_set_MSP_NS(uint32_t topOfStack) {
    __asm volatile ("MSR MSP_NS, %0" : : "r" (topOfStack) : );
}

#endif