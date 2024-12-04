/*****************************************************************************\
|   === extra.c : 2024 ===                                                    |
|                                                                             |
|    Extra functions for libC                                      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#define _SYSCALL_CREATE_FUNCS

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/************************
 *                     *
 *   DEBUG FUNCTIONS   *
 *                     *
************************/

int serial_debug(char *frm, ...) {
    va_list args;
    char *str;
    int len;

    va_start(args, frm);
    str = malloc(1024);

    len = vsprintf(str, frm, args);
    syscall_serial_write(SERIAL_PORT_A, str, len);

    free(str);
    va_end(args);

    return len;
}

void profan_print_memory(void *addr, uint32_t size) {
    for (uint32_t i = 0; i < size / 16 + (size % 16 != 0); i++) {
        printf("%08x: ", (uint32_t) addr + i * 16);

        for (int j = 0; j < 16; j++) {
            if (i * 16 + j < size)
                printf("%02x ", *((unsigned char *) addr + i * 16 + j));
            else
                printf("   ");
            if (j % 4 == 3)
                printf(" ");
        }

        for (int j = 0; j < 16; j++) {
            unsigned char c = *((unsigned char *) addr + i * 16 + j);
            if (i * 16 + j >= size)
                break;
            if (c >= 32 && c <= 126)
                printf("%c", c);
            else
                printf(".");
        }
        printf("\n");
    }
}

/*************************
 *                      *
 *     GLOBAL UTILS     *
 *                      *
*************************/

char *profan_join_path(const char *old, const char *new) {
    char *result;
    int len;

    if (new[0] == '/') {
        return strdup(new);
    }

    result = malloc(strlen(old) + strlen(new) + 2);
    strcpy(result, old);

    if (result[strlen(result) - 1] != '/') {
        strcat(result, "/");
    }
    strcat(result, new);

    len = strlen(result) - 1;
    if (result[len] == '/' && len > 0) {
        result[len] = '\0';
    }

    return result;
}

char *profan_input(int *size) {
    char *term = getenv("TERM");
    if (term && strstr(term, "serial"))
        return profan_input_serial(size, SERIAL_PORT_A);
    return profan_input_keyboard(size, term);
}

// defined in deluge - don't free libname and result
char *profan_fn_name(void *ptr, char **libname) {
    puts("libc extra: profan_fn_name: should not be called");
    return NULL;
}

struct stackframe {
  struct stackframe* ebp;
  uint32_t eip;
};

void profan_print_trace(void) {
    struct stackframe *ebp;

    asm volatile("movl %%ebp, %0" : "=r" (ebp));
    ebp = ebp->ebp;

    char *libname, *name;

    for (int i = 0; i < 5 && ebp; i++) {
        name = profan_fn_name((void *) ebp->eip, &libname);

        if (name)
            fprintf(stderr, "  %08x: %s (%s)\n", ebp->eip, name, libname);
        else
            fprintf(stderr, "  %08x: ???\n", ebp->eip);

        ebp = ebp->ebp;
    }
}

void profan_sep_path(const char *fullpath, char **parent, char **cnt) {
    int i, len;

    len = strlen(fullpath);

    if (parent != NULL) {
        *parent = calloc(1, len + 2);
    }

    if (cnt != NULL) {
        *cnt = calloc(1, len + 2);
    }

    while (len > 0 && fullpath[len - 1] == '/') {
        len--;
    }

    for (i = len - 1; i >= 0; i--) {
        if (fullpath[i] == '/') {
            break;
        }
    }

    if (parent != NULL && i >= 0) {
        if (i == 0) {
            strcpy(*parent, "/");
        } else {
            strncpy(*parent, fullpath, i);
        }
    }

    if (cnt != NULL) {
        strcpy(*cnt, fullpath + i + 1);
    }
}

/****************************
 *                         *
 *   KERNEL MEMORY ALLOC   *
 *                         *
****************************/

// kernel memory allocation functions
void *profan_kmalloc(uint32_t size, int as_kernel) {
    return (void *) syscall_mem_alloc(size, 0, as_kernel ? 6 : 1);
}

void *profan_kcalloc(uint32_t nmemb, uint32_t lsize, int as_kernel) {
    uint32_t size = lsize * nmemb;
    void *addr = (void *) syscall_mem_alloc(size, 0, as_kernel ? 6 : 1);

    if (addr == NULL)
        return NULL;

    memset(addr, 0, size);
    return addr;
}

void *profan_krealloc(void *mem, uint32_t new_size, int as_kernel) {
    if (mem == NULL)
        return (void *) syscall_mem_alloc(new_size, 0, as_kernel ? 6 : 1);

    uint32_t old_size = syscall_mem_get_alloc_size((uint32_t) mem);
    void *new_addr = (void *) syscall_mem_alloc(new_size, 0, as_kernel ? 6 : 1);

    if (new_addr == NULL)
        return NULL;

    memcpy(new_addr, mem, old_size < new_size ? old_size : new_size);
    syscall_mem_free((uint32_t) mem);
    return new_addr;
}

void profan_kfree(void *mem) {
    if (mem == NULL)
        return;
    syscall_mem_free((uint32_t) mem);
}
