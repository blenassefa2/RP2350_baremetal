#ifndef MPU_H
#define MPU_H


#define CONFIGURED_SIZE 3


#include <stdint.h>
#include <stdbool.h>

typedef struct
{
  volatile const  uint32_t TYPE;                   /*!< Offset: 0x000 (R/ )  MPU Type Register */
  volatile uint32_t CTRL;                   /*!< Offset: 0x004 (R/W)  MPU Control Register */
  volatile uint32_t RNR;                    /*!< Offset: 0x008 (R/W)  MPU Region Number Register */
  volatile uint32_t RBAR;                   /*!< Offset: 0x00C (R/W)  MPU Region Base Address Register */
  volatile uint32_t RLAR;                   /*!< Offset: 0x010 (R/W)  MPU Region Limit Address Register */
  volatile uint32_t RBAR_A1;                /*!< Offset: 0x014 (R/W)  MPU Region Base Address Register Alias 1 */
  volatile uint32_t RLAR_A1;                /*!< Offset: 0x018 (R/W)  MPU Region Limit Address Register Alias 1 */
  volatile uint32_t RBAR_A2;                /*!< Offset: 0x01C (R/W)  MPU Region Base Address Register Alias 2 */
  volatile uint32_t RLAR_A2;                /*!< Offset: 0x020 (R/W)  MPU Region Limit Address Register Alias 2 */
  volatile uint32_t RBAR_A3;                /*!< Offset: 0x024 (R/W)  MPU Region Base Address Register Alias 3 */
  volatile uint32_t RLAR_A3;                /*!< Offset: 0x028 (R/W)  MPU Region Limit Address Register Alias 3 */
           uint32_t RESERVED0[1];
  union {
    volatile uint32_t MAIR[2];
    struct {
        volatile uint32_t MAIR0;                  /*!< Offset: 0x030 (R/W)  MPU Memory Attribute Indirection Register 0 */
        volatile uint32_t MAIR1;                  /*!< Offset: 0x034 (R/W)  MPU Memory Attribute Indirection Register 1 */
    };
  };
} MPU_Type;

#define MPU_BASE    0xE000ED90

#define MPU         ((MPU_Type*)    MPU_BASE)





/** 
* @var policy::WA Write Allocation: Set to 1 to use cache allocation on write miss.
* @var policy::RA Read Allocation: Set to 1 to use cache allocation on read miss.
* @var policy::WB Write-Back: Set to 1 to use write-back update policy .
* @var policy::NT  Non-Transient: Set to 1 for non-transient data.
*/
typedef struct {
    unsigned WA     : 1; // bit 0
    unsigned RA     : 1; // bit 1
    unsigned WB     : 1; // bit 2  
    unsigned NT     : 1; // bit 3
}policy;

typedef struct  {
    policy outer;
    policy inner;
}normal_attribute;



/** 
 * G or nG – Gathering or non-Gathering. Multiple accesses to a device can be merged into a single
 * transaction except for operations with memory ordering semantics, for example, memory barrier
 * instructions, load acquire/store release.
 * R or nR – Reordering or Non-reordering.
 * E or nE – Early Write Acknowledge (similar to bufferable).
*/
struct device_attribute {
    unsigned ignore         : 2; // bit 0 - 1
    unsigned device_type    : 2; // bit 2 - 3 (00) nGnRnE , (01) nGnRE, (10) nGRE, (11) GRE
    unsigned ignore2        : 4; // bit 4 - 7
};

typedef enum  {
    nGnRnE,
    nGnRE,
    nGRE,
    GRE
}DEVICE_TYPE;



typedef struct {
    unsigned executable     :   1;  // bit 0      : (0) execution in this region allowed (1) not allowed
    unsigned wr         :   2;  // bit 1 - 2  : (00) Read/write by privileged code only, (01) r/w by any level, (10)ro by privileged code only, (11) ro by any level
    unsigned shareability    :   2;  // bit 3 - 4  : (00) Non-sharable, (01) Outershareable, (10) innershareable
    unsigned base_address   :   27; // bit 5 - 31 : Starting address of the MPU region
}rbar_s;

typedef enum  { 
    R_W_PRIVILEGED_ONLY,
    R_W_ANY_LEVEL,
    R_PRIVILEGED_ONLY,
    R_ANY_LEVEL
}WR;

typedef enum  {
    NON_SHAREABLE,
    OUTER_SHAREABLE,
    INNER_SHAREABLE
}SHARABILITY;


typedef struct  {
    unsigned region_enable  :   1;  // bit 0      : (0) not enabled (1) enabled
    unsigned attr_indx      :   3;  // bit 1 - 3  :  attribute sets from (0 - 3) MAIR0 and (4 - 7)MAIR1
    unsigned ignored        :   1;  // bit 4      : reserved bit for future  (set  to 0)
    unsigned limit_address  :   27; // bit 5 - 31 : Limiting address of the MPU region
}rlar_s;



typedef struct {
    rbar_s *base;
    rlar_s *limit;
    uint32_t rnr;
}mem_region;


void mpu_enable();
void mpu_disable();

void mpu_set_policy(policy * p, bool write_allocation, bool read_allocation, bool write_back, bool non_transient);
void mpu_set_normal_attribute(uint32_t attr_index, policy* outer, policy* inner);
void mpu_set_device_attribute(uint32_t attr_index, DEVICE_TYPE attr);
void mpu_set_rbar(rbar_s* base, bool executable, WR readability, SHARABILITY sharability, uint32_t base_address);
void mpu_set_rlar(rlar_s* limit, bool region_enable, uint32_t attr_indx, uint32_t limit_address);

void mpu_set_region(mem_region* region, rbar_s* base, rlar_s* limit, uint32_t rnr);
void mpu_unset_region(mem_region* region);




#endif