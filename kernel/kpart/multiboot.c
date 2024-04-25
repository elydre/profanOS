/****** This file is part of profanOS **************************\
|   == multiboot.c ==                                .pi0iq.    |
|                                                   d"  . `'b   |
|   Kernel Multiboot data handler                   q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

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
