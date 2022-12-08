#ifndef SETTING_ID
#define SETTING_ID 1002

#define get_func_addr ((int (*)(int, int)) *(int *) 0x199994)

// int sys_get_setting(char name[])

#define sys_get_setting ((int (*)(char[])) get_func_addr(SETTING_ID, 2))

#endif
