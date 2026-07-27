#include "flash.h"






typedef void *(*rom_table_lookup_fn)(uint32_t code, uint32_t mask);

static void *rom_func_lookup_inline(uint32_t code) {
        // on ARM the function pointer is stored in the table, so we dereference it
        // via lookup() rather than lookup_entry()
        rom_table_lookup_fn rom_table_lookup = (rom_table_lookup_fn) (uintptr_t)*(uint16_t*)(BOOTROM_TABLE_LOOKUP_OFFSET);
        
        return rom_table_lookup(code, RT_FLAG_FUNC_ARM_SEC);
  
}




static uint32_t boot2_copyout[BOOT2_SIZE_WORDS];
static bool boot2_copyout_valid = false;

static void __compiler_memory_barrier(void) {
    __asm ("" : : : "memory");
}

RAM_FUNC void flash_init_boot2_copyout(void) {
    if (boot2_copyout_valid)
        return;
  
    const volatile uint32_t *copy_from = (uint32_t *)BOOTRAM_BASE;

    for (int i = 0; i < BOOT2_SIZE_WORDS; ++i)
        boot2_copyout[i] = copy_from[i];
    __compiler_memory_barrier();
    boot2_copyout_valid = true;
}

RAM_FUNC void flash_enable_xip_via_boot2(void) {
    ((void (*)(void))((intptr_t)boot2_copyout+1))();
}





RAM_FUNC void flash_start_xip(void) {
    rom_connect_internal_flash_fn connect_internal_flash_func = (rom_connect_internal_flash_fn)rom_func_lookup_inline(ROM_FUNC_CONNECT_INTERNAL_FLASH);
    rom_flash_exit_xip_fn flash_exit_xip_func = (rom_flash_exit_xip_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_EXIT_XIP);
    rom_flash_flush_cache_fn flash_flush_cache_func = (rom_flash_flush_cache_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_FLUSH_CACHE);
    rom_flash_enter_cmd_xip_fn flash_enter_cmd_xip_func = (rom_flash_enter_cmd_xip_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_ENTER_CMD_XIP);
    
    if (!connect_internal_flash_func || !flash_exit_xip_func ||
        !flash_flush_cache_func || !flash_enter_cmd_xip_func)
    {
        while (1); // lookup failed — halt rather than jump into garbage
    }


    // Use ROM calls to get from ~any state to a state where low-speed flash access works:
    connect_internal_flash_func();
    flash_exit_xip_func();
    flash_flush_cache_func();
    flash_enter_cmd_xip_func();

    // If a boot2 is available then call it now. Slight limitation here is that if this is a
    // NO_FLASH binary which was loaded via bootrom LOAD_MAP, we should actually have a better
    // flash setup than this available via xip setup func stub left in boot RAM, but we can't
    // easily detect this case to take advantage of this.
    flash_init_boot2_copyout();
    flash_enable_xip_via_boot2();


}







RAM_FUNC void flash_range_erase(uint32_t flash_offs, uint32_t count) {
    
    if (!(flash_offs + count <= PICO_FLASH_SIZE_BYTES))
        while(1);
    rom_connect_internal_flash_fn connect_internal_flash_func = (rom_connect_internal_flash_fn)rom_func_lookup_inline(ROM_FUNC_CONNECT_INTERNAL_FLASH);
    rom_flash_exit_xip_fn flash_exit_xip_func = (rom_flash_exit_xip_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_EXIT_XIP);
    rom_flash_flush_cache_fn flash_flush_cache_func = (rom_flash_flush_cache_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_FLUSH_CACHE);
    rom_flash_range_erase_fn flash_range_erase_func = (rom_flash_range_erase_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_RANGE_ERASE);
        
    if (!connect_internal_flash_func || !flash_exit_xip_func ||
        !flash_flush_cache_func || !flash_range_erase_func)
    {
        while (1); // lookup failed — halt rather than jump into garbage
    }
       
    flash_init_boot2_copyout();
     
    // I am jumping this part because it is more for external additional flash memory (I think)
    // flash_hardware_save_state_t state;
    // flash_save_hardware_state(&state);
    // No flash accesses after this point
    __compiler_memory_barrier();

    connect_internal_flash_func();
    flash_exit_xip_func();
    flash_range_erase_func(flash_offs, count, FLASH_BLOCK_SIZE, FLASH_BLOCK_ERASE_CMD);
    flash_flush_cache_func(); // Note this is needed to remove CSn IO force as well as cache flushing
    
    flash_enable_xip_via_boot2();
}




RAM_FUNC void flash_range_program(uint32_t flash_offs, const uint8_t *data, uint32_t count)
{
    rom_connect_internal_flash_fn connect_internal_flash_func = (rom_connect_internal_flash_fn)rom_func_lookup_inline(ROM_FUNC_CONNECT_INTERNAL_FLASH);
    rom_flash_exit_xip_fn flash_exit_xip_func = (rom_flash_exit_xip_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_EXIT_XIP);
    rom_flash_range_program_fn flash_range_program_func = (rom_flash_range_program_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_RANGE_PROGRAM);
    rom_flash_flush_cache_fn flash_flush_cache_func = (rom_flash_flush_cache_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_FLUSH_CACHE);

    if (!connect_internal_flash_func || !flash_exit_xip_func ||
        !flash_flush_cache_func || !flash_range_program_func)
    {
        while (1); // lookup failed — halt rather than jump into garbage
    }

    flash_init_boot2_copyout();
    // I am jumping this part because it is more for external additional flash memory (I think)
    // flash_hardware_save_state_t state;
    // flash_save_hardware_state(&state);
    // No flash accesses after this point

    __compiler_memory_barrier();

    connect_internal_flash_func();
    flash_exit_xip_func();
    flash_range_program_func(flash_offs, data, count);
    flash_flush_cache_func(); // Note this is needed to remove CSn IO force as well as cache flushing
    
    flash_enable_xip_via_boot2();

    // flash_restore_hardware_state(&state);
}
