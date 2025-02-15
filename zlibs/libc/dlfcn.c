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

// real function are defined in the dynamic linker (/bin/x/deluge.elf)
// this file is just a stratagem to make the compiler happy

void *dlopen(const char *filename, int flag) {
    return (PROFAN_FNI, NULL);
}

void *dlsym(void *handle, const char *symbol) {
    return (PROFAN_FNI, NULL);
}

int dlclose(void *handle) {
    return (PROFAN_FNI, -1);
}

char *dlerror(void) {
    return (PROFAN_FNI, NULL);
}
