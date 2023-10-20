#ifndef SETJMP_ID
#define SETJMP_ID 1013

#include <type.h>

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

/*
int setjmp(jmp_buf var);
void longjmp(jmp_buf var,int m);
*/

typedef int jmp_buf[6];

#define setjmp ((int (*)(jmp_buf)) get_func_addr(SETJMP_ID, 2))
#define longjmp ((void (*)(jmp_buf, int)) get_func_addr(SETJMP_ID, 3))

#endif
