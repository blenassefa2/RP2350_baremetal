#ifndef UART_H
#define UART_H

#include <stdbool.h>

#include "debug.h"

#include <stdint.h>



#define UART0_BASE 0x40070000

#define UART0_UARTIBRD     (UART0_BASE + 0x24)
#define UART0_UARTFBRD     (UART0_BASE + 0x28)
#define UART0_UARTLCR_H    (UART0_BASE + 0x2c)
#define UART0_UARTCR       (UART0_BASE + 0x30)
#define UART0_UARTFR       (UART0_BASE + 0x18)
#define UART0_UARTDR       (UART0_BASE + 0x00)




#define UART_UARTFR_TXFF_BITS        (1u << 6)
#define UART_UARTFR_RXFE_BITS       (1u << 4)

#define UART_UARTCR_UARTEN_BITS     0x1
#define UART_UARTCR_TXE_BITS        0x100
#define UART_UARTCR_RXE_BITS        0x200







uint32_t initialize(uint32_t);
void uart_putc(char c);
void uart_puts(const char* s);
bool uart_is_readable(void);
char uart_getc(); // blocking read
#endif