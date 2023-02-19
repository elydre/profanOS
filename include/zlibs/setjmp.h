#ifndef SETJMP_ID
#define SETJMP_ID 1013

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

/*
int setjmp(jmp_buf var);
void longjmp(jmp_buf var,int m);
*/

typedef int jmp_buf[6];

#define setjmp ((int (*)(jmp_buf)) get_func_addr(SETJMP_ID, 2))
#define longjmp ((void (*)(jmp_buf, int)) get_func_addr(SETJMP_ID, 3))

#endif
