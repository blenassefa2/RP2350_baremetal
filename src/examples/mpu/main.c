// Copyright 2024 Mete Balci
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdbool.h>
#include <stdint.h>
#include "debug.h"
#include "mpu.h"



// defined in CMSIS as __StackTop
extern uint32_t _StackTop;



void Default_Handler(void);
void Reset_Handler(void);
void MemManage_Handler(void);
void NMI_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__ ((weak));
void BusFault_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void SecureFault_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__ ((weak, alias("Default_Handler")));

void Default_Handler(void)
{
  while (1);
}

void HardFault_Handler(void)
{
  configure_led();
  debug_blink(2);   // distinct pattern = "I crashed"
  while (1);
  
}
void MemManage_Handler(void)
{
  semihost_puts("\n Test Failed!!  mpu_region is set!!!\n");
  volatile uint32_t mmfsr = *(volatile uint32_t*)0xE000ED28 & 0xFF; // bottom byte = MMFSR
  volatile uint32_t mmfar = *(volatile uint32_t*)0xE000ED34;         // faulting address

  semihost_puts("\nMemManageFault at: ");
  semihost_put_hex(mmfar);
  semihost_puts("\nstatus: ");
  semihost_put_hex(mmfsr);
  semihost_puts("\n");
  while(1);  
}

void mpu_test( void ) {
   mpu_disable();

   /* Configure memory attributes */

   semihost_puts("\nAttribute not set: \n");
   semihost_put_hex(MPU->MAIR0);

   // Set Memory Attr 0
   
   // MPU_SetMemAttr(0UL, ARM_MPU_ATTR(
   //     ARM_MPU_ATTR_NON_CACHEABLE, // 0100 if write through and no allocation it is considered non cachable
   //     ARM_MPU_ATTR_NON_CACHEABLE
   // ));
   
  policy inner = {WA: 0, RA: 1, WB: 0, NT: 0};
  policy outer = {WA: 0, RA: 1, WB: 0, NT: 0};

  mpu_set_normal_attribute(0, &outer, &inner);
   

  semihost_puts("\nAttribute  set: \n");
  semihost_puts("\nMAIR0: ");
  semihost_put_hex32(MPU->MAIR0);
  semihost_puts("\nMAIR1: ");
  semihost_put_hex32(MPU->MAIR1);
   
   // Set Memory Attr 4

   // MPU_SetMemAttr(1UL, ARM_MPU_ATTR(
   //     ARM_MPU_ATTR_DEVICE,
   //     ARM_MPU_ATTR_DEVICE_nGnRE
   // ));
  mpu_set_device_attribute(6, nGnRE);
 
  semihost_puts("\nAttribute  set: \n");
  semihost_puts("\nMAIR0: ");
  semihost_put_hex32(MPU->MAIR0);
  semihost_puts("\nMAIR1: ");
  semihost_put_hex32(MPU->MAIR1);

  //  Initialize memory regions 
  mpu_set_device_attribute(1, nGnRnE);

  //  Configure region 0 - Mappable memory  0x00000000-0x00003FFF
  rbar_s base;

  mpu_set_rbar(&base, false, R_PRIVILEGED_ONLY, NON_SHAREABLE, 0x00000000);

  rlar_s limit;
  
  mpu_set_rlar(&limit, true, 1, 0x00003FFF);

  mem_region region;
  mpu_set_region(&region, &base, &limit, 0);

  
  mpu_enable();

  // This test should raise mem fault
  
  // test - 1 
    volatile int x = 0;
    uint32_t *ptr = (uint32_t *) 0x00003FF0;
    
    x = *ptr;   /* Read test  */
    *ptr = 0xA;  /* Write test: ptr is in region 0 which has read only  access */
  
  // test - 2
  mpu_disable();
  mpu_unset_region(&region);
  mpu_enable();

  // Because the region is unset it shouldn't memfault
  *ptr = 0xA;  /* Write test: ptr is in region 0 which has read only  access */

  semihost_puts("\nWrite-test passed!! becuse mpu_region is unset\n");
 
}


int main(void)
{

  configure_led();

  debug_blink(1); 

  mpu_test();

  
  // park
  while(1);
  return 0;
}

