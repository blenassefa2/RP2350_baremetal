#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include "debug.h"

#include "RP2350.h"
#include "core_cm33.h"
#include <stdint.h>






#define UART_UARTFR_TXFF_BITS        (1u << 6)
#define UART_UARTFR_RXFE_BITS       (1u << 4)

#define CLK_PERI_HZ 150000000u 

#define PLL_PWR_PD_BITS (1u << 0)
#define PLL_PWR_VCOPD_BITS   (1u << 5)
#define PLL_CS_LOCK_BITS          (1u << 31)
#define PLL_PWR_POSTDIVPD_BITS    (1u << 3)

#define PLL_PRIM_POSTDIV1_LSB     16u
#define PLL_PRIM_POSTDIV2_LSB     12u



uint32_t initialize(UART0_Type *, uint32_t);
void uart_putc(UART0_Type *uart, char c);
void uart_puts(UART0_Type *uart, const char* s);
bool uart_is_readable(UART0_Type *uart);
char uart_getc(UART0_Type *uart); // blocking read
#endif