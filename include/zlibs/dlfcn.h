#ifndef DLFCN_H
#define DLFCN_H

#define RTLD_LAZY 0
#define RTLD_NOW 1
#define RTLD_GLOBAL 2
#define RTLD_LOCAL 3
#define RTLD_NODELETE 4
#define RTLD_NOLOAD 5
#define RTLD_FATAL 6

void *dlopen(const char *filename, int flag);
void *dlsym(void *handle, const char *symbol);
int dlclose(void *handle);
char *dlerror(void);

#endif
