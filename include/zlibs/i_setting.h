#ifndef SETTING_ID
#define SETTING_ID 1002

#define get_func_addr ((int (*)(int, int)) *(int *) 0x199994)

// int setting_get(char name[])

#define setting_get ((int (*)(char[])) get_func_addr(SETTING_ID, 2))

#endif
