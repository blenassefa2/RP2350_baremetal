 .syntax unified
    .arch armv8-m.main
    .thumb

    .section .vectors, "ax", %progbits
    .align 11
    

    .global __Vectors
    .type __Vectors, %object
__Vectors:
    .word   __StackTop
    .word   Reset_Handler
    .word   NMI_Handler                            /* NMI_Handler */
    .word   HardFault_Handler
    .word   MemManage_Handler
    .word   BusFault_Handler                       /* BusFault_Handler */
    .word   UsageFault_Handler                     /* UsageFault_Handler */
    .word   SecureFault_Handler                    /* SecureFault_Handler */
    .word   0
    .word   0
    .word   0
    .word   SVC_Handler
    .word   DebugMon_Handler                       /* DebugMon_Handler */
    .word   0
    .word   PendSV_Handler                         /* PendSV_Handler */
    .word   SysTick_Handler                        /* SysTick_Handler */
    /* Interrupts */
    .long    Default_Handler //Interrupt0_HANDLER                /*   0 Interrupt 0 */
    .long    Default_Handler //Interrupt1_Handler                 /*   1 Interrupt 1 */
    .long    Default_Handler //Interrupt2_Handler                 /*   2 Interrupt 2 */
    .long    Default_Handler //Interrupt3_Handler                 /*   3 Interrupt 3 */
    .long    Default_Handler //Interrupt4_Handler                 /*   4 Interrupt 4 */
    .long    Default_Handler //Interrupt5_Handler                 /*   5 Interrupt 5 */
    .long    Default_Handler //Interrupt6_Handler                 /*   6 Interrupt 6 */
    .long    Default_Handler //Interrupt7_Handler                 /*   7 Interrupt 7 */
    .long    Default_Handler //Interrupt8_Handler                 /*   8 Interrupt 8 */
    .long    Default_Handler //Interrupt9_Handler                 /*   9 Interrupt 9 */

    .space   (470 * 4)                          /* Interrupts 10 .. 480 are left out */


.section .picobin_block, "a"
    .word 0xffffded3          // PICOBIN_BLOCK_MARKER_START

    // item 0: IMAGE_TYPE
    .byte 0x42
    .byte 0x1
    .hword 0b0001000000100001

    // item 1: LAST
    .byte 0xff
    .hword 0x0001
    .byte 0

    .word 0                  // relative ptr to next block (0 = self)
    .word 0xab123579          // PICOBIN_BLOCK_MARKER_END

.section .text, "ax", %progbits
    .align 2
    .global Reset_Handler               
    .thumb_func
Reset_Handler:
    bl     .Lcopy_data
    bl     .Lzero_bss
    b      main

    .align 2
    .thumb_func
.Lcopy_data:
    ldr     r0, =__data_source__
    ldr     r1, =__data_start__
    ldr     r2, =__data_end__
.Lcopy_data_loop:
    cmp     r1, r2
    bcs     .Lcopy_data_done
    ldr     r3, [r0], #4
    str     r3, [r1], #4
    b       .Lcopy_data_loop
.Lcopy_data_done:
    bx      lr

    .thumb_func
.Lzero_bss:
    ldr     r0, =__bss_start__
    ldr     r1, =__bss_end__
    movs    r2, #0
.Lzero_bss_loop:
    cmp     r0, r1
    bcs     .Lzero_bss_done
    str     r2, [r0], #4
    b       .Lzero_bss_loop
.Lzero_bss_done:
    bx      lr


