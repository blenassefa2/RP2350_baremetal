// SPDX-License-Identifier: GPL-3.0-or-later

#include "uart.h"
#include "clock.h"
#include "resets.h"
#include "gpio.h"
#include "debug.h"

/** 
 * pll, clock and uart initialization as well as uart related functionalities
 *
 * Comments in this file document relevant portions of the original pico-sdk
 * implementation and explain the changes made to eliminate the SDK dependency.
 */

uint32_t uart_set_baudrate(uint32_t baudrate) {
  // I'll jump this validation cause I trust my code for now
  // valid_params_if(HARDWARE_UART, UART_IS_INSTANCE(uart));

  // Also jumping this (too much to worry for small implementation but  will look in to it later)
  // if (uart_clock_get_hz(uart) == 0) {
  //     return 0;
  // }

  // This uart_clock_get_hz could be replaced with clock_get_hz(clk_peri);
  // the clk_peri is a global variable from hardware/clocks.h in the pico sdk
  // uint32_t baud_rate_div = (8 * uart_clock_get_hz(uart) / baudrate) + 1;
  uint32_t baud_rate_div = (8 * get_clk_sys() / baudrate) + 1;
  uint32_t baud_ibrd = baud_rate_div >> 7;
  uint32_t baud_fbrd;

  if (baud_ibrd == 0) {
      baud_ibrd = 1;
      baud_fbrd = 0;
  } else if (baud_ibrd >= 65535) {
      baud_ibrd = 65535;
      baud_fbrd = 0;
  }  else {
      baud_fbrd = (baud_rate_div & 0x7f) >> 1;
  }

  *(volatile uint32_t *)(UART0_UARTIBRD) = baud_ibrd;
  *(volatile uint32_t *)(UART0_UARTFBRD) = baud_fbrd;

  // PL011 needs a (dummy) LCR_H write to latch in the divisors.
  // I don't want to actually change LCR_H contents here.
  // I don't need this because in the uart_init there is already writting on lcr_h
  // uart_write_lcr_bits_masked(uart, 0, 0);

  // See datasheet
  return (4 * CLK_PERI_HZ) / (64 * baud_ibrd + baud_fbrd);
}
void uart_reset(void)
{
    // SET alias: bit 26 -> 1, all other bits in RESET untouched
    *(volatile uint32_t *)(RESETS_RESET + REG_ALIAS_SET_BITS) = RESETS_UART0_BIT;
}

void uart_unreset(void)
{
    // CLR alias: bit 26 -> 0, all other bits in RESET untouched
    *(volatile uint32_t *)(RESETS_RESET + REG_ALIAS_CLR_BITS) = RESETS_UART0_BIT;

    while (~ *(volatile uint32_t *)(RESETS_RESET_DONE) & RESETS_UART0_BIT);

   
}


uint32_t initialize(uint32_t baudrate)
{

    debug_blink(1);
    clk_init();

    debug_blink(1);
  
    uart_reset();

    debug_blink(1);
    uart_unreset();

    debug_blink(1);
    // set baud rate
    uint32_t baud = uart_set_baudrate(baudrate);
    debug_blink(1);


    // fix the configuration of the uart hardware
    // uint32_t data_bits = 8;
    // uint32_t stop_bits = 1;
    // uint parity = UART_PARITY_NONE;
    // hw_write_masked(&uart_get_hw(uart)->lcr_h,
    //     ((data_bits - 5u) << UART_UARTLCR_H_WLEN_LSB) |
    //         ((stop_bits - 1u) << UART_UARTLCR_H_STP2_LSB) |
    //         (bool_to_bit(parity != UART_PARITY_NONE) << UART_UARTLCR_H_PEN_LSB) |
    //         (bool_to_bit(parity == UART_PARIY_EVEN) << UART_UARTLCR_H_EPS_LSB) |
    //         UART_UARTLCR_H_FEN_BITS,
    //     UART_UARTLCR_H_WLEN_BITS | UART_UARTLCR_H_STP2_BITS |
    //         UART_UARTLCR_H_PEN_BITS | UART_UARTLCR_H_EPS_BITS |
    //         UART_UARTLCR_H_FEN_BITS);
    *(volatile uint32_t *)(UART0_UARTLCR_H) = 0x70;

    // Enable the UART, both TX and RX
    *(volatile uint32_t *)(UART0_UARTCR) = UART_UARTCR_UARTEN_BITS | UART_UARTCR_TXE_BITS | UART_UARTCR_RXE_BITS; // 0x301


    *(volatile uint32_t *)(IO_BANK0_GPIO0_CTRL) = GPIO_FUNCSEL_UART;     // funcsel 2 = UART0 TX
    *(volatile uint32_t *)(PADS_BANK0_GPIO0)    = 0x40;     // IE=1 (input enable), rest cleared including ISO
  
    *(volatile uint32_t *)(IO_BANK0_GPIO1_CTRL) = GPIO_FUNCSEL_UART;     // funcsel 2 = UART0 RX
    *(volatile uint32_t *)(PADS_BANK0_GPIO1)    = 0x40;     // IE=1 — mandatory here, or RX reads nothing
  
    return baud;

}

bool uart_is_writable() {
  return !(*(volatile uint32_t *)(UART0_UARTFR) & UART_UARTFR_TXFF_BITS);
}

void uart_write_blocking(const char *src, uint32_t len) {
  
  for (uint32_t i = 0; i < len; ++i)
  {
    while (!uart_is_writable())
      ;
      *(volatile uint32_t *)(UART0_UARTDR) = *src++;
  }  
  
}

bool uart_is_readable() {
  // PL011 doesn't expose levels directly, so return values are only 0 or 1
  return !(*(volatile uint32_t *)(UART0_UARTFR) & UART_UARTFR_RXFE_BITS);
}

void uart_read_blocking(char *dst, uint32_t len) {
  for (uint32_t i = 0; i < len; ++i) {
      while (!uart_is_readable());
      *dst++ = (char)(*(volatile uint32_t *)(UART0_UARTDR));
  }
}

void uart_putc(char c)
{
  uart_write_blocking(&c, 1);

}
void uart_puts(const char* s)
{
  while (*s) uart_putc(*s++);
}

char uart_getc() {
  char c;
  uart_read_blocking((char *) &c, 1);
  return c;
}

void uart_put_hex_byte(uint8_t byte)
{
    static const char hex_digits[] = "0123456789abcdef";
    char out[3];

    out[0] = hex_digits[(byte >> 4) & 0x0F];
    out[1] = hex_digits[byte & 0x0F];
    out[2] = '\0';

    uart_puts(out);
}