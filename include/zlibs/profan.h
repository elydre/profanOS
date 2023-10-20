#ifndef PROFAN_LIB_ID
#define PROFAN_LIB_ID 1002

#include <type.h>

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define assemble_path ((void (*)(char *, char *, char *)) get_func_addr(PROFAN_LIB_ID, 2))
#define profan_print_stacktrace ((void (*)(void)) get_func_addr(PROFAN_LIB_ID, 3))
#define profan_print_memory ((void (*)(void *, uint32_t)) get_func_addr(PROFAN_LIB_ID, 4))
#define profan_kb_load_map ((int (*)(char *)) get_func_addr(PROFAN_LIB_ID, 5))
#define profan_kb_get_char ((char (*)(uint8_t, uint8_t)) get_func_addr(PROFAN_LIB_ID, 6))

#endif
