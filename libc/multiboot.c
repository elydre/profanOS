#include <libc/multiboot.h>

/* 00 -> flags
 * 01 -> mem_lower
 * 02 -> mem_upper
 * 03 -> boot_device
 * 04 -> cmdline
 * 05 -> mods_count
 * 06 -> mods_addr
 * 07 -> num
 * 08 -> size
 * 09 -> addr
 * 10 -> shndx
 * 11 -> mmap_length
 * 12 -> mmap_addr
 * 13 -> drives_length
 * 14 -> drives_addr
 * 15 -> config_table
 * 16 -> boot_loader_name
 * 17 -> apm_table
 * 18 -> vbe_control_info
 * 19 -> vbe_mode_info
 * 20 -> vbe_mode
 * 21 -> vbe_interface_seg
 * 22 -> vbe_interface_off
 * 23 -> vbe_interface_len

 * a part of the multiboot data can be 0 if the
 * kernel is not loaded by a multiboot loader */

uint32_t multiboot_data[24];

void mboot_save(void *mboot_ptr) {
    for (uint32_t i = 0; i < 24; ++i) {
        multiboot_data[i] = ((uint32_t *) mboot_ptr)[i];
    }
}

uint32_t mboot_get(int index) {
    return multiboot_data[index];
}
