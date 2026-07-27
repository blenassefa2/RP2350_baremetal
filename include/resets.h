#ifndef RESETS_H
#define RESETS_H

#define RESETS_BASE 0x40020000
#define RESETS_RESET (RESETS_BASE + 0x0)
#define RESETS_RESET_DONE (RESETS_BASE + 0x8)

// Register address offsets for atomic RMW aliases
#define REG_ALIAS_SET_BITS 0x2000u
#define REG_ALIAS_CLR_BITS 0x3000u


#define RESETS_UART0_BIT      (1u << 26)
#define RESETS_PLL_BIT        (1u << 14)
#define RESET_IO_BANK0_BITS   (1u << 6)
#define RESET_PADS_BANK0_BITS (1u << 9)


#endif