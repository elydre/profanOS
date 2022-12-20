#ifndef DEMO_ID
#define DEMO_ID 1005

#define get_func_addr ((int (*)(int, int)) *(int *) 0x199994)

/*
int main();
void init_lib();
void demo_func();
*/

#define demo_func ((void (*)(int)) get_func_addr(DEMO_ID, 3))

#endif
