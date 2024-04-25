/****** This file is part of profanOS **************************\
|   == dlfcn.c ==                                    .pi0iq.    |
|                                                   d"  . `'b   |
|   Implementation of dlfcn functions for libc      q. /|\  u   |
|   see details below                                `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#include <profan/type.h>

// real function are defined in the dynamic linker (/bin/sys/deluge)
// this file is just a stratagem to make the compiler happy

void *dlopen(const char *filename, int flag) {
    return NULL;
}

void *dlsym(void *handle, const char *symbol) {
    return NULL;
}

int dlclose(void *handle) {
    return -1;
}

char *dlerror(void) {
    return NULL;
}
