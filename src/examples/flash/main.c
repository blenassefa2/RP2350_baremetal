// Copyright 2024 Mete Balci
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdbool.h>
#include <stdint.h>


#include "flash.h"  
#include "uart.h"
#include "debug.h"

extern uint32_t _StackTop;



void Default_Handler(void);
void Reset_Handler(void);
void NMI_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__ ((weak));
void MemManage_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void SecureFault_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__ ((weak, alias("Default_Handler")));

void Default_Handler(void)
{
  while (1);
}

void HardFault_Handler(void)
{
  configure_led();
  debug_blink(2);   // distinct pattern = "I crashed"
  while (1);
  
}



#define FLASH_TARGET_OFFSET (256u * 1024u)

static const uint8_t *flash_target_contents =
    (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);


static void print_buf(const uint8_t *buf, uint32_t len)
{
    for (uint32_t i = 0; i < len; ++i)
    {
        semihost_put_hex(buf[i]);
        if (i % 16 == 15)
            uart_puts("\n");
        else
        semihost_puts(" ");
    }
}


static uint32_t lcg_state = 0x12345678u;

static uint8_t next_random_byte(void)
{
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return (uint8_t)(lcg_state >> 16);
}


int main(void)
{

    configure_led();

    debug_blink(1); // LED set_up confirmation

    uint8_t random_data[FLASH_PAGE_SIZE];
    for (unsigned i = 0; i < FLASH_PAGE_SIZE; ++i)
        random_data[i] = next_random_byte();

    semihost_puts("Generated data:\n");
    // print_buf(random_data, FLASH_PAGE_SIZE);

    semihost_puts("\nErasing...\n");

    // Assumption: single core, no interrupts enabled,
    // nothing else can be executing from flash concurrently
    // if not then it is best to wrap with a safe flash execute function 
    // that makes sure no other concurrent program is trying to update or read from the flash
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    
    // Assumption: single core, no interrupts enabled,
    // nothing else can be executing from flash concurrently
    // if not then it is best to wrap with a safe flash execute function 
    // that makes sure no other concurrent program is trying to update or read from the flash
    // flash_start_xip();

    semihost_puts("Done:\n\n");
    // print_buf(flash_target_contents, FLASH_PAGE_SIZE);

    semihost_puts("\nProgramming... \n");
    
    flash_range_program(FLASH_TARGET_OFFSET, random_data, FLASH_PAGE_SIZE);
    // flash_start_xip();

    semihost_puts("Done:\n");
    // print_buf(flash_target_contents, FLASH_PAGE_SIZE);

    semihost_puts("\nErasing...\n");

     // Assumption: single core, no interrupts enabled,
    // nothing else can be executing from flash concurrently
    // if not then it is best to wrap with a safe flash execute function 
    // that makes sure no other concurrent program is trying to update or read from the flash
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    // flash_start_xip();

    semihost_puts("Done:\n\n");
    print_buf(flash_target_contents, FLASH_PAGE_SIZE);

    semihost_puts("\nProgramming... \n");

    flash_range_program(FLASH_TARGET_OFFSET, random_data, FLASH_PAGE_SIZE);
    // flash_start_xip();

    semihost_puts("Done:\n");
    print_buf(flash_target_contents, FLASH_PAGE_SIZE);

    bool mismatch = false;
    for (unsigned i = 0; i < FLASH_PAGE_SIZE; ++i)
    {
        if (random_data[i] != flash_target_contents[i])
            mismatch = true;
    }

    if (mismatch)
        semihost_puts("Content doesn't match!! Programming failed!\n");
    else
        semihost_puts("Content matches!! Programming successful!\n");

    // park
    while (1);
}