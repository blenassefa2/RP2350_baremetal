
#include "mpu.h"
#include "cortex_m33.h"
#include "debug.h"


static inline uint32_t RLAR(rlar_s* rl)
{
    return (rl->limit_address << 5 ) | (rl->attr_indx << 1) | rl->region_enable;
}

static inline uint32_t RBAR(rbar_s* rb)
{
    return (rb->base_address << 5) | (rb->shareability << 3) | (rb->wr << 1) | rb->executable;
}

void mpu_enable() 
{
    __DMB();
    
    MPU->CTRL |= 5;             /* Enable the MPU */
    SCB->SHCSR |= (1 << 16);    /* enable MemManage fault */
    
    __DSB();                    /* Force memory writes before continuing */
    __ISB();                    /* Flush and refill pipeline with updated permissions */
}
void mpu_disable()
{
    __DMB();
    
    MPU->CTRL |= 0;             /* Disable the MPU */
 
    __DSB();                    /* Force memory writes before continuing */
    __ISB();                    /* Flush and refill pipeline with updated permissions */
}


void mpu_set_policy(policy * p, bool write_allocation, bool read_allocation, bool write_back, bool non_transient)
{
    p->WA = (uint32_t)write_allocation;
    p->RA = (uint32_t)read_allocation;
    p->WB = (uint32_t)write_back;
    p->NT = (uint32_t)non_transient;
}

void mpu_set_normal_attribute(uint32_t attr_index, policy* outer, policy* inner)
{
    semihost_puts("\nattr_index: "); semihost_put_hex(attr_index);
    if (attr_index < 0) {
        return; // invalid index
    }

    const uint32_t idx = attr_index;
    const uint32_t reg = idx / 4U;
    const uint32_t pos = (idx % 4U) * 8U;
    const uint32_t mask = 0xFFU << pos;

    
    const uint8_t inner_nibble = (uint8_t)(((inner->NT & 1U) << 3U) | ((inner->WB & 1U) << 2U) |
                                            ((inner->RA & 1U) << 1U) | (inner->WA & 1U));
    const uint8_t outer_nibble = (uint8_t)(((outer->NT & 1U) << 3U) | ((outer->WB & 1U) << 2U) |
                                            ((outer->RA & 1U) << 1U) | (outer->WA & 1U));
    
    if (reg >= (sizeof(MPU->MAIR) / sizeof(MPU->MAIR[0]))) {
        return; // invalid index
    }
    const uint8_t attr = (uint8_t)((outer_nibble << 4U) | inner_nibble);

    semihost_puts("\nreg: "); semihost_put_hex(reg);
    semihost_puts("\npos: "); semihost_put_hex(pos);
    semihost_puts("\nattr: "); semihost_put_hex(attr);

    MPU->MAIR[reg] = (MPU->MAIR[reg] & ~mask) | ((attr << pos) & mask);
}

void mpu_set_device_attribute(uint32_t attr_index, DEVICE_TYPE type)
{
    if (attr_index < 0) {
        return; // invalid index
    }

    const uint32_t idx = (uint32_t)attr_index;
    const uint32_t reg = idx / 4U;
    const uint32_t pos = (idx % 4U) * 8U;
    const uint32_t mask = 0xFFU << pos;
   
    if (reg >= (sizeof(MPU->MAIR) / sizeof(MPU->MAIR[0]))) {
        return; // invalid index
    }
    const uint8_t attr = (uint8_t)((type & 0x3U) << 2U);

    semihost_puts("reg: "); semihost_put_hex(reg);
    semihost_puts(" pos: "); semihost_put_hex(pos);
    semihost_puts(" attr: "); semihost_put_hex(attr);

    MPU->MAIR[reg] = (MPU->MAIR[reg] & ~mask) |  ((attr << pos) & mask);
}


void mpu_set_rbar(rbar_s* base, bool executable, WR readability, SHARABILITY shareability, uint32_t base_address)
{
    base->executable   = (uint32_t)executable    &1U;
    base->wr           = (uint32_t)readability   &3U;
    base->shareability = (uint32_t)shareability  &3U;
    base->base_address = (base_address           &0x7FFFFFFU) >> 5;
}

void mpu_set_rlar(rlar_s* limit, bool region_enable, uint32_t attr_indx, uint32_t limit_address)
{
    limit->region_enable  = (uint32_t)region_enable   &1U;
    limit->attr_indx      = attr_indx                 &7U;
    limit->limit_address  = (limit_address            &0x7FFFFFFU) >> 5;
}

void mpu_set_region(mem_region* region, rbar_s* base, rlar_s* limit, uint32_t rnr)
{
    region->base = base;
    region->limit = limit;
    region->rnr = rnr;
    
    MPU->RNR = rnr;
    MPU->RBAR = RBAR(base);
    MPU->RLAR = RLAR(limit);
}
void mpu_unset_region(mem_region* region)
{
    MPU->RNR = region->rnr;
    MPU->RLAR = 0U;
    MPU->RBAR = 0U;
}


