#ifndef LIB_ID
#define LIB_ID 123

#define get_func_addr ((int (*)(int, int)) *(int *) 0x199994)

/*
void print_version();
*/

#define lib_print_version ((void (*)()) get_func_addr(LIB_ID, 2))

#endif
