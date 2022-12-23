#ifndef STDIO_ID
#define STDIO_ID 1009

#include <type.h>

#define stdin 0
#define stdout 1
#define stderr 2
#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

/* https://en.cppreference.com/w/c/io/tmpnam for documentation */
#define TMP_MAX 14776336
#define TMP_MAX_S 14776336
#define L_tmpnam 4
#define L_tmpnam_s 4

#define EOF -1
#define FOPEN_MAX 1024
#define FILENAME_MAX 20
#define BUFSIZ 1024 // TODO : CHOSE A CORRECT VALUE
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2


#define get_func_addr ((int (*)(int, int)) *(int *) 0x199994)

/*
int main();
*/


#endif
