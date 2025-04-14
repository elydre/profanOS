/*****************************************************************************\
|   === profan.c : 2024 ===                                                   |
|                                                                             |
|    Extra functions for libC                                      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#define _SYSCALL_CREATE_FUNCS

#include <profan/filesys.h>
#include <profan/syscall.h>
#include <profan.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>

#include "config_libc.h"

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

    len = sprintf(str, "[%d] ", syscall_process_pid());
    len += vsprintf(str + len, frm, args);

    syscall_serial_write(SERIAL_PORT_A, str, len);

    free(str);
    va_end(args);

    return len;
}

// defined in deluge (dynamic linker)
__attribute__((weak)) void *dlg_fn_name(void *ptr, char **libname);

char *profan_fn_name(void *ptr, char **libname) {
    return dlg_fn_name(ptr, libname);
}

void profan_print_trace(void) {
    struct stackframe {
        struct stackframe *ebp;
        uint32_t eip;
    } *ebp;

    asm volatile("movl %%ebp, %0" : "=r" (ebp));
    ebp = ebp->ebp;

    char *libname, *name;

    for (int i = 0; i < 8 && ebp; i++) {
        if (ebp->eip >= 0xB0000000 && ebp->eip < 0xC0000000)
            break; // deluge

        name = dlg_fn_name((void *) ebp->eip, &libname);

        if (name)
            fprintf(stderr, "  %08x: %s (%s)\n", ebp->eip, name, libname);
        else
            fprintf(stderr, "  %08x: ???\n", ebp->eip);

        ebp = ebp->ebp;
    }
}

/*************************
 *                      *
 *     GLOBAL UTILS     *
 *                      *
*************************/

char *profan_input(int *size) {
    char *term = getenv("TERM");
    if (term && strstr(term, "serial"))
        return profan_input_serial(size, SERIAL_PORT_A);
    return profan_input_keyboard(size, term);
}

void profan_nimpl(const char *name) {
    fprintf(stderr, "libc: %s: function not implemented\n", name);
    #if NOT_IMPLEMENTED_ABORT
    profan_print_trace();
    abort();
    #endif
}

char *profan_libc_version(void) {
    return PROFAN_LIBC_VERSION;
}

/*************************
 *                      *
 *   FILESYSTEM UTILS   *
 *                      *
*************************/

extern uint32_t g_wd_sid;
extern char g_wd_path[PATH_MAX];

uint32_t profan_wd_sid(void) {
    return g_wd_sid;
}

const char *profan_wd_path(void) {
    return g_wd_path;
}

char *profan_path_join(const char *old, const char *new) {
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

void profan_path_sep(const char *fullpath, char **parent, char **cnt) {
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

uint32_t profan_path_resolve(const char *path) {
    if (path[0] == '/')
        return fu_path_to_sid(SID_ROOT, path);
    return fu_path_to_sid(profan_wd_sid(), path);
}

char *profan_path_path(const char *exec, int allow_path) {
    // resolve executable path from PATH with extension support
    // ls -> /bin/cmd/ls.elf  -  returns NULL if not found
    uint32_t file_sid;

    if (strchr(exec, '/')) {
        if (!allow_path) {
            errno = EACCES;
            return NULL;
        }
        char *file_path = profan_path_join(profan_wd_path(), exec);
        fu_simplify_path(file_path);
        if (fu_path_to_sid(SID_ROOT, file_path))
            return file_path;
        errno = ENOENT;
        return NULL;
    }

    char *dir, *path = strdup(getenv("PATH")); // OK with NULL

    if (path)
        dir = strtok(path, ":");

    else if ((dir = path = getcwd(NULL, 0)) == NULL) {
        errno = ENOENT;
        return NULL;
    }

    while (dir != NULL) {
        uint32_t size, dir_sid = profan_path_resolve(dir);
        uint8_t *buf = NULL;

        if (!fu_is_dir(dir_sid))
            goto endloop;

        int count = fu_dir_get_size(dir_sid);
        if (count < 0)
            goto endloop;

        size = syscall_fs_get_size(NULL, dir_sid);
        if (size == UINT32_MAX || size < sizeof(uint32_t))
            goto endloop;

        buf = malloc(size);
        if (syscall_fs_read(NULL, dir_sid, buf, 0, size))
            goto endloop;

        for (int i = 0; i < count; i++) {
            int offset = fu_dir_get_elm(buf, size, i, &file_sid);

            if (offset <= 0)
                break;

            if (!fu_is_file(file_sid))
                continue;

            char *name = (char *) buf + offset;

            int j;
            for (j = 0; exec[j] && name[j] && exec[j] == name[j]; j++);
            if (exec[j] == '\0' && (name[j] == '.' || name[j] == '\0')) {
                char *r = profan_path_join(dir, name);
                free(path);
                free(buf);
                return r;
            }
        }

        endloop:
        free(buf);
        dir = strtok(NULL, ":");
    }

    free(path);

    errno = ENOENT;
    return NULL;
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
