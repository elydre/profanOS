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

#define RTLD_LAZY     0
#define RTLD_NOW      1
#define RTLD_GLOBAL   2
#define RTLD_LOCAL    3
#define RTLD_NODELETE 4
#define RTLD_NOLOAD   5
#define RTLD_FATAL    6

#define RTLD_DEFAULT ((void *) 0)

void *dlopen(const char *filename, int flag);
void *dlsym(void *handle, const char *symbol);
int   dlclose(void *handle);
char *dlerror(void);

#endif
