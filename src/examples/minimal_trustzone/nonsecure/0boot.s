    .syntax unified
    .arch armv8-m.main
    .thumb

    .section .vectors, "ax", %progbits
    .align 7
    .global __Vectors
__Vectors:
    .word __StackTop
    .word Reset_Handler

.section .text, "ax", %progbits
    .align 2
    .global Reset_Handler
    .thumb_func
Reset_Handler:
    bl main
inf_loop:
    b inf_loop