// Copyright 2024 Mete Balci
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdbool.h>
#include <stdint.h>

#include "RP2350.h"
#include "core_cm33.h"
#include "debug.h"
#include "uart.h"

// On Pico2, LED is connected to GPIO25

// defined in CMSIS as __StackTop
extern uint32_t __INITIAL_SP;

extern __NO_RETURN void _start(void);

void Default_Handler(void);
__NO_RETURN void Reset_Handler(void);
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

#define LED_PIN 19


// defined in system_RP2350.h
typedef void(*VECTOR_TABLE_Type)(void);
// __VECTOR_TABLE is __Vectors (m-profile/cmsis_gcc.h)
extern const VECTOR_TABLE_Type __VECTOR_TABLE[496];
// attribute is defined as (used, section(".vectors"))
const VECTOR_TABLE_Type __VECTOR_TABLE[496] __VECTOR_TABLE_ATTRIBUTE = {
  (VECTOR_TABLE_Type)(&__INITIAL_SP),
  Reset_Handler,
  NMI_Handler,
  HardFault_Handler,
  MemManage_Handler,
  BusFault_Handler,
  UsageFault_Handler,
  SecureFault_Handler,
  0,
  0,
  0,
  SVC_Handler,
  DebugMon_Handler,
  0,
  PendSV_Handler,
  SysTick_Handler
};

void Default_Handler(void)
{
  while (1);
}

__NO_RETURN void Reset_Handler(void)
{
  // defined in CMSIS as __StackLimit
  extern uint32_t __STACK_LIMIT;

  __set_PSP((uint32_t)(&__INITIAL_SP));
  __set_MSPLIM((uint32_t)(&__STACK_LIMIT));
  __set_PSPLIM((uint32_t)(&__STACK_LIMIT));

  // copy data section from FLASH to RAM
  extern uint32_t __data_start__;
  extern uint32_t __data_end__;
  extern uint32_t __data_source__;

  uint32_t* p = &__data_start__;
  uint32_t* q = &__data_source__;

  while (p < &__data_end__)
  {
    *p++ = *q++;
  }

  // start newlib._start in newlib/libc/sys/arm/crt0.S
  _start();
}

void HardFault_Handler(void)
{
  

  while (1)
  {
    debug_blink(10);

  }
}

__STATIC_FORCEINLINE void configure_led()
{
  // function 5 = SIO 
  IO_BANK0->GPIO19_CTRL = 0x05;
  PADS_BANK0->GPIO19 = 0x34;
  // enable output
  SIO->GPIO_OE_SET = 0x01U << LED_PIN;
}

__STATIC_FORCEINLINE void turn_led_on()
{
  SIO->GPIO_OUT_SET = 0x01U << LED_PIN;
}

__STATIC_FORCEINLINE void turn_led_off()
{
  SIO->GPIO_OUT_CLR = 0x01U << LED_PIN;
}

__STATIC_FORCEINLINE void flip_led()
{
  SIO->GPIO_OUT_XOR = 0x01U << LED_PIN;
}

#define UART_ID UART0
#define BAUD_RATE 115200

#define WAIT_TIME 400000u
int main(void)
{

  configure_led();
  turn_led_off();
  

  debug_blink(1);
  
 

  initialize(UART_ID, BAUD_RATE);

  // gpio_set_function is basically updating the io pins but with more abstraction
  // simply it is like  PADS_BANK0->GPIO15 in the configure led in this current code
  // 
  // Set the TX and RX pins by using the function select on the GPIO
  // Set datasheet for more information on function select
  // gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
  // gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));

  IO_BANK0->GPIO0_CTRL = 0x02;     // funcsel 2 = UART0 TX
  PADS_BANK0->GPIO0    = 0x40;     // IE=1 (input enable), rest cleared including ISO

  IO_BANK0->GPIO1_CTRL = 0x02;     // funcsel 2 = UART0 RX
  PADS_BANK0->GPIO1    = 0x40;     // IE=1 — mandatory here, or RX reads nothing



  // Use some the various UART functions to send out data
  // In a default system, printf will also output via the default UART

  // Send out a character without any conversions
  // uart_putc_raw(UART_ID, 'A');

  // Send out a character but do CR/LF conversions
  // uart_putc( UART_ID, 'B');

  // Send out a string, with CR/LF conversions
  // uart_puts(UART_ID, " Hello, UART!\n");

  // while (true) 
  // {
  //   if (uart_is_readable(UART_ID)) {
  //       char c = uart_getc(UART_ID);
  //       if (c == '\r' || c == '\n') {
  //           turn_led_on();
  //           uart_puts(UART_ID, "Confirmed!!!!!\r\n");
  //           for (int i = 0; i < WAIT_TIME; i++) {
  //               ;
  //           }
  //           turn_led_off();
  //       }
  //   }

    
  // }
  const char *buff = "Hello Semihosting!!";
  int result = semihost_write_byte(buff,  19) ;

  if (result == 0)
  {
    uart_puts(UART_ID, "Success!");
    
  }
  else
  {
    uart_puts(UART_ID, "Failure!\n");
    debug_blink(result);
  }

  while(1);
  return 0;
}