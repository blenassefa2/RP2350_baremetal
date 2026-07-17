// SPDX-License-Identifier: GPL-3.0-or-later

#include "uart.h"



uint32_t uart_set_baudrate( UART0_Type *uart, uint32_t baudrate) {
  // I'll jump this validation cause I trust my code for now
  // valid_params_if(HARDWARE_UART, UART_IS_INSTANCE(uart));

  // Also jumping this (too much to worry for small implementation but  will look in to it later)
  // if (uart_clock_get_hz(uart) == 0) {
  //     return 0;
  // }

  // This uart_clock_get_hz could be replaced with clock_get_hz(clk_peri);
  // the clk_peri is a global variable from hardware/clocks.h in the pico sdk
  // uint32_t baud_rate_div = (8 * uart_clock_get_hz(uart) / baudrate) + 1;
  uint32_t baud_rate_div = (8 * CLK_PERI_HZ / baudrate) + 1;
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

  uart->UARTIBRD = baud_ibrd;
  uart->UARTFBRD = baud_fbrd;

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
    *(volatile uint32_t *)((uintptr_t)&RESETS->RESET + REG_ALIAS_SET_BITS) = RESETS_UART0_BIT;
}

void uart_unreset(void)
{
    // CLR alias: bit 26 -> 0, all other bits in RESET untouched
    *(volatile uint32_t *)((uintptr_t)&RESETS->RESET + REG_ALIAS_CLR_BITS) = RESETS_UART0_BIT;

    while (~RESETS->RESET_DONE & RESETS_UART0_BIT);

   
}
uint32_t initialize(UART0_Type *uart, uint32_t baudrate)
{


    // SETUP THE CLOCK CRYSTAL
    // In the pico SDK this is not done in the uart_init instead it was done somewhere else
    // but I noticed the effect of its absence when the hardware is truly reset and I try to run serial communication
    // and it didn't work.
    // Start the 12 MHz crystal
    XOSC->CTRL = 0xaa0;
    XOSC->STARTUP = 47;
    XOSC->CTRL |= (0xfabU << 12);
    while (!(XOSC->STATUS & (1u << 31)));

    // Feed clk_peri from the crystal directly (bypasses PLL/clk_sys entirely)
    CLOCKS->CLK_PERI_CTRL = (4u << 5) | (1u << 11);
  
    // Reset  (on pico sdk it does this 
    // I guess it was not really on high (set) 
    // so we have to do reset first then unreset it for real)
    uart_reset();
    uart_unreset();


    // set baud rate
    uint32_t baud = uart_set_baudrate((UART0_Type *)uart, baudrate);


    // fix the configuration of the uart hardware (the struct UART0_type)
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
    uart->UARTLCR_H = 0x70;

    // Enable the UART, both TX and RX
    // uart_get_hw(uart)->cr = UART_UARTCR_UARTEN_BITS | UART_UARTCR_TXE_BITS | UART_UARTCR_RXE_BITS;
    uart->UARTCR = 0x301;

    return baud;

}

bool uart_is_writable(UART0_Type *uart) {
  return !(uart->UARTFR & UART_UARTFR_TXFF_BITS);
}

void uart_write_blocking(UART0_Type *uart, const char *src, uint32_t len) {
  
  for (uint32_t i = 0; i < len; ++i)
  {
    while (!uart_is_writable(uart))
      ;
    uart->UARTDR = *src++;
  }  
  
}

bool uart_is_readable(UART0_Type *uart) {
  // PL011 doesn't expose levels directly, so return values are only 0 or 1
  return !(uart->UARTFR & UART_UARTFR_RXFE_BITS);
}

void uart_read_blocking(UART0_Type  *uart, char *dst, uint32_t len) {
  for (uint32_t i = 0; i < len; ++i) {
      while (!uart_is_readable(uart));
      *dst++ = (char) uart->UARTDR;
  }
}

void uart_putc(UART0_Type *uart, char c)
{
  uart_write_blocking(uart, &c, 1);

}
void uart_puts(UART0_Type *uart, const char* s)
{
  while (*s) uart_putc( uart, *s++);
}

char uart_getc(UART0_Type *uart) {
  char c;
  uart_read_blocking(uart, (char *) &c, 1);
  return c;
}