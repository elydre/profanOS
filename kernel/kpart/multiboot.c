/*****************************************************************************\
|   === multiboot.c : 2024 ===                                                |
|                                                                             |
|    Kernel Multiboot data handler                                 .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/multiboot.h>

/* a part of the multiboot data can be 0 if the
 * kernel is not loaded by a multiboot loader */

uint32_t multiboot_data[32];

void mboot_save(void *mboot_ptr) {
    for (uint32_t i = 0; i < 32; ++i) {
        multiboot_data[i] = ((uint32_t *) mboot_ptr)[i];
    }
}

uint32_t mboot_get(int index) {
    return multiboot_data[index];
}
