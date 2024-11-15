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
#define PROFAN_LIB_ID 1005

#include <profan/type.h>

#define run_ifexist(path, argc, argv) \
        run_ifexist_full((runtime_args_t){path, argc, argv, __get_environ_ptr(), 1}, NULL)

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

int   serial_debug(char *frm, ...);
void  profan_print_memory(void *addr, uint32_t size);

char *profan_join_path(const char *old, const char *new);
void  profan_sep_path(const char *fullpath, char **parent, char **cnt);

char *profan_input(int *size);
int   profan_wait_pid(uint32_t pid);
char *profan_fn_name(void *ptr, char **libname);

void *profan_kmalloc(uint32_t size, int as_kernel);
void *profan_kcalloc(uint32_t nmemb, uint32_t lsize, int as_kernel);
void *profan_krealloc(void *mem, uint32_t new_size, int as_kernel);
void  profan_kfree(void *mem);

#ifndef PROFAN_C
#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define userspace_reporter ((int (*)(char *)) get_func_addr(PROFAN_LIB_ID, 2))
#define profan_kb_load_map ((int (*)(char *)) get_func_addr(PROFAN_LIB_ID, 3))
#define profan_kb_get_char ((char (*)(uint8_t, uint8_t)) get_func_addr(PROFAN_LIB_ID, 4))
#define profan_input_keyboard ((char *(*)(int *, char *)) get_func_addr(PROFAN_LIB_ID, 5))
#define profan_input_serial ((char *(*)(int *, int)) get_func_addr(PROFAN_LIB_ID, 6))
#define run_ifexist_full ((int (*)(runtime_args_t, int *)) get_func_addr(PROFAN_LIB_ID, 7))
#endif

#endif
