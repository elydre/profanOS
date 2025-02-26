/*****************************************************************************\
|   === multiboot.h : 2024 ===                                                |
|                                                                             |
|    Kernel Multiboot header                                       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <ktype.h>

// https://www.gnu.org/software/grub/manual/multiboot/html_node/Boot-information-format.html

typedef struct {
    uint32_t flags;             //  0 Indicates which fields are valid 
    uint32_t mem_lower;         //  4 Lower memory in KB 
    uint32_t mem_upper;         //  8 Upper memory in KB 
    uint32_t boot_device;       // 12 Boot device
    uint32_t cmdline;           // 16 Address of the command line
    uint32_t mods_count;        // 20 Number of loaded modules
    uint32_t mods_addr;         // 24 Address of the modules

    struct {
        uint32_t num;           // 28 Number of entries
        uint32_t size;          // 32 Size of an entry
        uint32_t addr;          // 36 Address of the ELF table
        uint32_t shndx;         // 40 Index of the string table
    } elf_sec;

    uint32_t mmap_length;       // 44 Length of memory map entries
    uint32_t mmap_addr;         // 48 Address of memory map entries

    uint32_t drives_length;     // 52 Length of drive structures
    uint32_t drives_addr;       // 56 Address of drive structures

    uint32_t config_table;      // 60 Address of the ROM configuration table
    uint32_t boot_loader_name;  // 64 Address of the boot loader name

    uint32_t apm_table;         // 68 Address of the APM table

    uint32_t vbe_control_info;  // 72 Address of the VBE control info
    uint32_t vbe_mode_info;     // 76 Address of the VBE mode info
    uint16_t vbe_mode;          // 80 VBE mode
    uint16_t vbe_interface_seg; // 82 VBE interface segment
    uint16_t vbe_interface_off; // 84 VBE interface offset
    uint16_t vbe_interface_len; // 86 VBE interface length

    uint32_t fb_addr_low;       //  88 Address of the framebuffer
    uint32_t fb_addr_high;      //  92 Address of the framebuffer
    uint32_t fb_pitch;          //  96 Pitch (bytes per line)
    uint32_t fb_width;          // 100 Width in pixels
    uint32_t fb_height;         // 104 Height in pixels
    uint8_t  fb_bpp;            // 108 Bits per pixel
    uint8_t  fb_type;           // 109 Type of framebuffer
    uint8_t  color_info[6];     // 110 Color information
} __attribute__((packed)) multiboot_t;

extern multiboot_t *g_mboot;

#endif
