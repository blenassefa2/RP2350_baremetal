#ifndef FLASH_H
#define FLASH_H


#include  <stdint.h>
#include <stdbool.h>
// To replace the __no_inline_not__in_flash_func macro and avoid issues of programming from flash while updating it
// Currently, since the functions are using ROM routines, the flash is used
// But later, for preventing issues, the linker.ld should be updated to keep this section on the RAM
#define RAM_FUNC __attribute__((noinline, section(".ram_functions")))



#define FLASH_PAGE_SIZE (1u << 8)
#define FLASH_SECTOR_SIZE (1u << 12)
#define FLASH_BLOCK_SIZE (1u << 16)

#define PICO_FLASH_SIZE_BYTES (8 * 1024 * 1024)

#define BOOTROM_FUNC_TABLE_OFFSET 0x14
#define BOOTROM_WELL_KNOWN_PTR_SIZE 2
#define BOOTROM_TABLE_LOOKUP_OFFSET     (BOOTROM_FUNC_TABLE_OFFSET + BOOTROM_WELL_KNOWN_PTR_SIZE)

#define RT_FLAG_FUNC_ARM_SEC    0x0004

#define BOOT2_SIZE_WORDS 64
#define BOOTRAM_BASE 0x400e0000

#define XIP_BASE            0x10000000u


#define ROM_TABLE_CODE(c1, c2) ((c1) | ((c2) << 8))
#define ROM_FUNC_CONNECT_INTERNAL_FLASH         ROM_TABLE_CODE('I', 'F')
#define ROM_FUNC_FLASH_EXIT_XIP                 ROM_TABLE_CODE('E', 'X')
#define ROM_FUNC_FLASH_FLUSH_CACHE              ROM_TABLE_CODE('F', 'C')
#define ROM_FUNC_FLASH_ENTER_CMD_XIP            ROM_TABLE_CODE('C', 'X')
#define ROM_FUNC_FLASH_RANGE_ERASE              ROM_TABLE_CODE('R', 'E')
#define ROM_FUNC_FLASH_RANGE_PROGRAM            ROM_TABLE_CODE('R', 'P')

#define FLASH_BLOCK_ERASE_CMD 0xd8

typedef void (*rom_connect_internal_flash_fn)(void);
typedef void (*rom_flash_exit_xip_fn)(void);
typedef void (*rom_flash_flush_cache_fn)(void);
typedef void (*rom_flash_enter_cmd_xip_fn)(void);
typedef void (*rom_flash_range_erase_fn)(uint32_t, uint32_t, uint32_t, uint8_t);
typedef void (*rom_flash_range_program_fn)(uint32_t, const uint8_t*, uint32_t);


RAM_FUNC void flash_start_xip(void);
RAM_FUNC void flash_range_erase(uint32_t flash_offs, uint32_t count);
RAM_FUNC void flash_range_program(uint32_t flash_offs, const uint8_t *data, uint32_t count);

#endif