// Copyright 2024 Mete Balci
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdbool.h>
#include <stdint.h>
#include "debug.h"
#include "uart.h"
// #include "flash.h"

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

#define BAUD_RATE 115200

#define WAIT_TIME 400000u

int main(void)
{

  configure_led();

  debug_blink(1); // LED set_up confirmation

  initialize(BAUD_RATE);

  debug_blink(1); // UART set_up confirmation
 


  // ------------ Serial Line Testing -----------
  // Use some the various UART functions to send out data

  // Send out a character but do CR/LF conversions
  uart_putc('B');

  // Send out a string, with CR/LF conversions
  uart_puts( " Hello, UART!\n");

  while (true) 
  {
    if (uart_is_readable()) {
        char c = uart_getc();
        
        // Send confirmation message if Enter key is recieved on the serial line
        
        if (c == '\r' || c == '\n') {
            turn_led_on();
            uart_puts("Confirmed!!!!!\r\n");
            for (int i = 0; i < WAIT_TIME; i++) {
                ;
            }
            turn_led_off();
        }
    }

    
  }


  
  // park
  while(1);
  return 0;
}

