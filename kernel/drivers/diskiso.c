/*****************************************************************************\
|   === diskiso.c : 2024 ===                                                  |
|                                                                             |
|    Kernel Disk ISO (grub module) functions                       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/snowflake.h>
#include <kernel/multiboot.h>
#include <minilib.h>
#include <system.h>

uint32_t diskiso_start;
uint32_t diskiso_size;

int init_diskiso(void) {
    uint32_t mod_end, end_pos, start;

    diskiso_size = 0;

    if (!mboot_get(6)) {
        return 1;
    }

    start = *(uint32_t *) mboot_get(6);
    diskiso_size = (*(uint32_t *) (mboot_get(6) + 4) - start) / 256;

    diskiso_start = MEM_BASE_ADDR + PARTS_COUNT * sizeof(allocated_part_t) + 1;

    mod_end = start + diskiso_size * 256;
    end_pos = diskiso_start + diskiso_size * 256;

    // copy the module to the new position starting from the end
    for (uint32_t i = 0; i <= diskiso_size * 256; i++) {
        *((uint8_t *) (end_pos - i)) = *((uint8_t *) (mod_end - i));
    }

    return 0;
}

uint32_t diskiso_get_size(void) {
    return diskiso_size * 256;
}

void *diskiso_get_start(void) {
    return (void *) diskiso_start;
}

void diskiso_free(void) {
    diskiso_size = 0;
    free((void *) diskiso_start);
}
