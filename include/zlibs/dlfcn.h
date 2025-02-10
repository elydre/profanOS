/*****************************************************************************\
|   === dlfcn.h : 2024 ===                                                    |
|                                                                             |
|    Implementation of the dlfcn.h header file from libC           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _DLFCN_H
#define _DLFCN_H

#define RTLD_LAZY     1
#define RTLD_NOW      2
#define RTLD_LOCAL    0x000
#define RTLD_GLOBAL   0x100
#define RTLD_TRACE    0x200
#define RTLD_NODELETE 0x400
#define RTLD_NOLOAD   0x800

#define RTLD_NEXT     ((void *) -1)   // search subsequent objects
#define RTLD_DEFAULT  ((void *) -2)   // search in RTLD_GLOBAL

void *dlopen(const char *filename, int flag);
void *dlsym(void *handle, const char *symbol);
int   dlclose(void *handle);
char *dlerror(void);

#endif
