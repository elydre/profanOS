#ifndef SETJMP_H
#define SETJMP_H

typedef int jmp_buf[6];

int setjmp(jmp_buf var);
void longjmp(jmp_buf var, int m);

#endif
