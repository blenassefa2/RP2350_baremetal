__S_FLASH_BASE   = 0x10000000;
__S_FLASH_SIZE   = 0x3F000;      /* 252K, code+rodata */
__NSC_FLASH_BASE = 0x1003F000;
__NSC_FLASH_SIZE = 0x1000;       /* 4K, veneers only */

__S_SRAM_BASE = 0x20000000;
__S_SRAM_SIZE = 256K;

__HEAP_SIZE  = 16K;
__STACK_SIZE = 16K;

MEMORY
{
  FLASH (rx)  : ORIGIN = __S_FLASH_BASE,   LENGTH = __S_FLASH_SIZE
  NSC   (rx)  : ORIGIN = __NSC_FLASH_BASE, LENGTH = __NSC_FLASH_SIZE
  RAM   (rwx) : ORIGIN = __S_SRAM_BASE,    LENGTH = __S_SRAM_SIZE
}

PHDRS
{
  text PT_LOAD FLAGS(5);
  nsc  PT_LOAD FLAGS(5);
  data PT_LOAD FLAGS(6);
  bss  PT_LOAD FLAGS(6);
}

SECTIONS
{
  .text :
  {
    . = ALIGN(4);
    KEEP(*(.vectors))
    KEEP(*(.picobin_block))
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH :text

  /* CMSE veneer stubs (SG instructions) — must land exactly in the NSC region */
  .gnu.sgstubs :
  {
    . = ALIGN(32);
    KEEP(*(.gnu.sgstubs*))
  } >NSC :nsc

  . = ALIGN(4);
  __data_source__ = .;

  .data :
  {
    __data_start__ = .;
    *(.data*)
    . = ALIGN(4);
    __data_end__ = .;
  } >RAM AT> FLASH :data

  .bss :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    *(.bss*)
    . = ALIGN(4);
    __bss_end__ = .;
  } >RAM :bss

  .heap (COPY) :
  {
    . = ALIGN(4);
    . = . + __HEAP_SIZE;
    __HeapLimit = .;
  } > RAM

  .stack (ORIGIN(RAM) + LENGTH(RAM) - __STACK_SIZE) (COPY) :
  {
    . = ALIGN(4);
    __StackLimit = .;
    . = . + __STACK_SIZE;
    . = ALIGN(4);
    __StackTop = .;
  } > RAM

  ASSERT(__StackLimit >= __HeapLimit, "stack or heap overflow")
}