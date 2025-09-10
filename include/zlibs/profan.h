/*****************************************************************************\
|   === profan.h : 2024 ===                                                   |
|                                                                             |
|    Usefull functions for profanOS (wiki/lib_profan)              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef PROFAN_LIB_ID
#define PROFAN_LIB_ID 4

#include <profan/minimal.h>
#include <stdint.h>

_BEGIN_C_FILE

typedef struct {
    char *path;     // path to file
    char *wd;       // working directory

    int argc;       // argument count
    char **argv;    // argument list
    char **envp;    // environment list

    uint8_t sleep_mode;  // sleep mode
} runtime_args_t;

#define KB_ESC      1
#define KB_BACK     14
#define KB_TAB      15
#define KB_ENTER    28
#define KB_CTRL     29
#define KB_LSHIFT   42
#define KB_RSHIFT   54
#define KB_TOP      72
#define KB_LEFT     75
#define KB_RIGHT    77
#define KB_BOT      80
#define KB_DEL      83
#define KB_RVAL     128
#define KB_RESEND   224

#define KB_A        16
#define KB_Q        30
#define KB_E        18
#define KB_R        19
#define KB_D        32
#define KB_Z        17
#define KB_S        31

#ifndef _KERNEL_MODULE

#define PROFAN_FNI profan_nimpl(__FUNCTION__)

int   serial_debug(char *frm, ...);
char *profan_fn_name(void *ptr, char **libname);
void  profan_print_trace(void);

void  profan_nimpl(const char *name);
char *profan_libc_version(void);

char     *profan_path_join(const char *old, const char *new);
void      profan_path_sep(const char *fullpath, char **parent, char **cnt);
char     *profan_path_path(const char *exec, int allow_path);
uint32_t  profan_path_resolve(const char *path);
int       profan_path_simplify(char *path);


uint32_t    profan_wd_sid(void);
const char *profan_wd_path(void);

void *profan_kmalloc(uint32_t size, int as_kernel);
void *profan_kcalloc(uint32_t nmemb, uint32_t lsize, int as_kernel);
void *profan_krealloc(void *mem, uint32_t new_size, int as_kernel);
void  profan_kfree(void *mem);

#endif

// kernel module interface

int profan_kb_load_map(char *map);
char profan_kb_get_char(uint8_t scancode, uint8_t shift);
char *profan_input_keyboard(int *scancode, char *buf);
int run_ifexist(runtime_args_t *args, int *pid);

#ifndef _KERNEL_MODULE

extern int profan_syscall(uint32_t id, ...);

#undef  _pscall
#define _pscall(module, id, ...) \
    profan_syscall(((module << 24) | id), __VA_ARGS__)

#define profan_kb_load_map(a)       ((int) _pscall(PROFAN_LIB_ID, 0, a))
#define profan_kb_get_char(a, b)    ((char) _pscall(PROFAN_LIB_ID, 1, a, b))
#define profan_input_keyboard(a, b) ((char *) _pscall(PROFAN_LIB_ID, 2, a, b))
#define run_ifexist(a, b)           ((int) _pscall(PROFAN_LIB_ID, 3, a, b))

#else
#ifndef PROFAN_C

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define profan_kb_load_map ((int (*)(char *)) get_func_addr(PROFAN_LIB_ID, 0))
#define profan_kb_get_char ((char (*)(uint8_t, uint8_t)) get_func_addr(PROFAN_LIB_ID, 1))
#define profan_input_keyboard ((char *(*)(int *, char *)) get_func_addr(PROFAN_LIB_ID, 2))
#define run_ifexist ((int (*)(runtime_args_t *, int *)) get_func_addr(PROFAN_LIB_ID, 3))

#endif
#endif

_END_C_FILE

#endif
