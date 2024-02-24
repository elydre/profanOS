#ifndef PROFAN_LIB_ID
#define PROFAN_LIB_ID 1002

#include <type.h>

#define run_ifexist(path, argc, argv) \
        run_ifexist_full((runtime_args_t){path, (sid_t){0, 0}, \
        argc, argv, 0, 1}, NULL)

#define KB_LEFT 75
#define KB_RIGHT 77
#define KB_OLDER 72
#define KB_NEWER 80
#define KB_CTRL 29
#define KB_ESC 1
#define KB_A 16
#define KB_Q 30
#define KB_E 18
#define KB_R 19
#define KB_D 32
#define KB_Z 17
#define KB_S 31
#define KB_released_value 128

#define O_RDONLY    00
#define O_WRONLY    01
#define O_RDWR      02
#define O_CREAT     0100
#define O_TRUNC     01000
#define O_APPEND    02000

#ifndef PROFAN_C
#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define userspace_reporter ((int (*)(char *)) get_func_addr(PROFAN_LIB_ID, 2))
#define assemble_path ((char * (*)(char *, char *)) get_func_addr(PROFAN_LIB_ID, 3))
#define profan_print_stacktrace ((void (*)(void)) get_func_addr(PROFAN_LIB_ID, 4))
#define profan_print_memory ((void (*)(void *, uint32_t)) get_func_addr(PROFAN_LIB_ID, 5))
#define profan_kb_load_map ((int (*)(char *)) get_func_addr(PROFAN_LIB_ID, 6))
#define profan_kb_get_char ((char (*)(uint8_t, uint8_t)) get_func_addr(PROFAN_LIB_ID, 7))
#define profan_wait_pid ((int (*)(uint32_t)) get_func_addr(PROFAN_LIB_ID, 8))
#define open_input_keyboard ((char *(*)(int *, char *)) get_func_addr(PROFAN_LIB_ID, 9))
#define open_input_serial ((char *(*)(int *, int)) get_func_addr(PROFAN_LIB_ID, 10))
#define open_input ((char *(*)(int *)) get_func_addr(PROFAN_LIB_ID, 11))
#define serial_debug ((int (*)(char *, ...)) get_func_addr(PROFAN_LIB_ID, 12))
#define profan_open ((int (*)(char *, int, ...)) get_func_addr(PROFAN_LIB_ID, 13))
#define run_ifexist_full ((int (*)(runtime_args_t, int *)) get_func_addr(PROFAN_LIB_ID, 14))

#endif

#endif
