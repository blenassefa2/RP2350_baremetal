#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include "debug.h"

#include "RP2350.h"
#include "core_cm33.h"
#include <stdint.h>




// Register address offsets for atomic RMW aliases
#define REG_ALIAS_SET_BITS 0x2000u
#define REG_ALIAS_CLR_BITS 0x3000u


#define RESETS_UART0_BIT      (1u << 26)

#define UART_UARTFR_TXFF_BITS        (1u << 6)
#define UART_UARTFR_RXFE_BITS       (1u << 4)

#define CLK_PERI_HZ 12000000u 




uint32_t initialize(UART0_Type *, uint32_t);
void uart_putc(UART0_Type *uart, char c);
void uart_puts(UART0_Type *uart, const char* s);
bool uart_is_readable(UART0_Type *uart);
char uart_getc(UART0_Type *uart); // blocking read
#endif