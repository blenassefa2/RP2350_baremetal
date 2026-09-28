__NS_RAM_BASE = 0x20040000;
__NS_RAM_SIZE = 256K;
__STACK_SIZE  = 4K;

MEMORY { RAM (rwx) : ORIGIN = __NS_RAM_BASE, LENGTH = __NS_RAM_SIZE }

SECTIONS
{
  .text :
  {
    . = ALIGN(4);
    KEEP(*(.vectors))
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
  } > RAM

  .data : { *(.data*) . = ALIGN(4); } > RAM
  .bss  : { *(.bss*)  . = ALIGN(4); } > RAM

  .stack (ORIGIN(RAM) + LENGTH(RAM) - __STACK_SIZE) (COPY) :
  {
    . = . + __STACK_SIZE;
    __StackTop = .;
  } > RAM
}