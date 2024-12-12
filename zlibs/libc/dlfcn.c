/*****************************************************************************\
|   === dlfcn.c : 2024 ===                                                    |
|                                                                             |
|    Implementation of dlfcn functions from libC                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan.h>
#include <stddef.h>

// real function are defined in the dynamic linker (/bin/sys/deluge)
// this file is just a stratagem to make the compiler happy

void *dlopen(const char *filename, int flag) {
    profan_nimpl("dlopen");
    return NULL;
}

void *dlsym(void *handle, const char *symbol) {
    profan_nimpl("dlsym");
    return NULL;
}

int dlclose(void *handle) {
    profan_nimpl("dlclose");
    return -1;
}

char *dlerror(void) {
    profan_nimpl("dlerror");
    return NULL;
}
