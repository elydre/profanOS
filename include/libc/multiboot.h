#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <type.h>


void mboot_save(void *mboot_ptr);
uint32_t mboot_get(int index);

#endif
