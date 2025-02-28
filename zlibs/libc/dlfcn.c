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

#undef WEAK
#define WEAK __attribute__((weak))

WEAK void *dlg_open(const char *filename, int flag);
WEAK int   dlg_close(void *handle);
WEAK void *dlg_sym(void *handle, const char *symbol);
WEAK char *dlg_error(void);

void *dlopen(const char *filename, int flag) {
    return dlg_open(filename, flag);
}

int dlclose(void *handle) {
    return dlg_close(handle);
}

void *dlsym(void *handle, const char *symbol) {
    return dlg_sym(handle, symbol);
}

char *dlerror(void) {
    return dlg_error();
}
