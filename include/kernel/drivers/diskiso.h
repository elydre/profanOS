#ifndef DISKISO_H
#define DISKISO_H

#include <type.h>

int      init_diskiso();

uint32_t diskiso_get_size();
uint32_t diskiso_get_start();
void     diskiso_free();

#endif
