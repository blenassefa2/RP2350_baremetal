#ifndef RAND_H
#define RAND_H

#include <stdint.h>
#include <stdbool.h>



#define TRNG_BASE       0x400f0000

#define TRNG_RNG_IMR_OFFSET 0x00000100


typedef struct {

    volatile uint32_t rng_imr;
   
    const volatile uint32_t rng_isr;
   
    volatile uint32_t rng_icr;

  
    
    volatile uint32_t trng_config;

   
   
    const volatile uint32_t trng_valid;


    const volatile uint32_t ehr_data[6];


   
    volatile uint32_t rnd_source_enable;

 
   
    volatile uint32_t sample_cnt1;


   
    volatile uint32_t autocorr_statistic;

   
    
    volatile uint32_t trng_debug_control;

    uint32_t _pad0;

 
   
    volatile uint32_t trng_sw_reset;

    uint32_t _pad1[28];

    
  
    volatile uint32_t rng_debug_en_input;

    const volatile uint32_t trng_busy;

    volatile uint32_t rst_bits_counter;

    const volatile uint32_t rng_version;
    uint32_t _pad2[7];

    const volatile uint32_t rng_bist_cntr[3];
} trng_hw_t;

#define trng_hw ((trng_hw_t *)(TRNG_BASE + TRNG_RNG_IMR_OFFSET))

typedef struct rng_128 {
    uint64_t r[2];
} rng_128_t;

#ifndef count_of
#define count_of(a) (sizeof(a)/sizeof((a)[0]))
#endif

void initialise_trng(); // crypt_init

uint32_t get_rand_32();
uint64_t get_rand_64();
void get_rand_128(rng_128_t *rand128);

#endif