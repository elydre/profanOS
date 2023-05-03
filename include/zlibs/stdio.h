#ifndef STDIO_ID
#define STDIO_ID 1009

#include <type.h>

#define stdin (FILE *) 0
#define stdout (FILE *) 1
#define stderr (FILE *) 2

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

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif
#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

// we dont define functions if we are in the .c file
#ifndef STDIO_C
#define clearerr ((void (*)(FILE *)) get_func_addr(STDIO_ID, 3))
#define fopen ((FILE * (*)(const char *, const char *)) get_func_addr(STDIO_ID, 4))
#define fopen_s ((errno_t (*)(FILE * *, const char *, const char *)) get_func_addr(STDIO_ID, 5))
#define freopen ((FILE * (*)(const char *, const char *, FILE *)) get_func_addr(STDIO_ID, 6))
#define freopen_s ((errno_t (*)(FILE * *, const char *, const char *, FILE *)) get_func_addr(STDIO_ID, 7))
#define fclose ((int (*)(FILE *)) get_func_addr(STDIO_ID, 8))
#define fflush ((int (*)(FILE *)) get_func_addr(STDIO_ID, 9))
#define setbuf ((void (*)(FILE *, char *)) get_func_addr(STDIO_ID, 10))
#define setvbuf ((int (*)(FILE *, char *, int, size_t)) get_func_addr(STDIO_ID, 11))
#define fwide ((int (*)(FILE *, int)) get_func_addr(STDIO_ID, 12))
#define fread ((size_t (*)(void *, size_t, size_t, FILE *)) get_func_addr(STDIO_ID, 13))
#define fwrite ((size_t (*)(const void *, size_t, size_t, FILE *)) get_func_addr(STDIO_ID, 14))
#define fgetc ((int (*)(FILE *)) get_func_addr(STDIO_ID, 15))
#define getc ((int (*)(FILE *)) get_func_addr(STDIO_ID, 16))
#define fgets ((char * (*)(char *, int, FILE *)) get_func_addr(STDIO_ID, 17))
#define fputc ((int (*)(int, FILE *)) get_func_addr(STDIO_ID, 18))
#define putc ((int (*)(int, FILE *)) get_func_addr(STDIO_ID, 19))
#define fputs ((int (*)(const char *, FILE *)) get_func_addr(STDIO_ID, 20))
#define getchar ((int (*)(void)) get_func_addr(STDIO_ID, 21))
#define gets_s ((errno_t (*)(char *, rsize_t)) get_func_addr(STDIO_ID, 22))
#define putchar ((int (*)(int)) get_func_addr(STDIO_ID, 23))
#define puts ((int (*)(const char *)) get_func_addr(STDIO_ID, 24))
#define ungetc ((int (*)(int, FILE *)) get_func_addr(STDIO_ID, 25))
#define scanf ((int (*)(const char *, ...)) get_func_addr(STDIO_ID, 26))
#define fscanf ((int (*)(FILE *, const char *, ...)) get_func_addr(STDIO_ID, 27))
#define sscanf ((int (*)(const char *, const char *, ...)) get_func_addr(STDIO_ID, 28))
#define scanf_s ((int (*)(const char *, ...)) get_func_addr(STDIO_ID, 29))
#define fscanf_s ((int (*)(FILE *, const char *, ...)) get_func_addr(STDIO_ID, 30))
#define sscanf_s ((int (*)(const char *, const char *, ...)) get_func_addr(STDIO_ID, 31))
#define vscanf ((int (*)(const char *, va_list)) get_func_addr(STDIO_ID, 32))
#define vfscanf ((int (*)(FILE *, const char *, va_list)) get_func_addr(STDIO_ID, 33))
#define vsscanf ((int (*)(const char *, const char *, va_list)) get_func_addr(STDIO_ID, 34))
#define vscanf_s ((int (*)(const char *, va_list)) get_func_addr(STDIO_ID, 35))
#define vfscanf_s ((int (*)(FILE *, const char *, va_list)) get_func_addr(STDIO_ID, 36))
#define vsscanf_s ((int (*)(const char *, const char *, va_list)) get_func_addr(STDIO_ID, 37))
#define printf ((int (*)(const char *, ...)) get_func_addr(STDIO_ID, 38))
#define fprintf ((int (*)(FILE *, const char *, ...)) get_func_addr(STDIO_ID, 39))
#define sprintf ((int (*)(char *, const char *, ...)) get_func_addr(STDIO_ID, 40))
#define snprintf ((int (*)(char *, size_t, const char *, ...)) get_func_addr(STDIO_ID, 41))
#define printf_s ((int (*)(const char *, ...)) get_func_addr(STDIO_ID, 42))
#define fprintf_s ((int (*)(FILE *, const char *, ...)) get_func_addr(STDIO_ID, 43))
#define sprintf_s ((int (*)(char *, rsize_t, const char *, ...)) get_func_addr(STDIO_ID, 44))
#define snprintf_s ((int (*)(char *, rsize_t, rsize_t, const char *, ...)) get_func_addr(STDIO_ID, 45))
#define vprintf ((int (*)(const char *, va_list)) get_func_addr(STDIO_ID, 46))
#define vfprintf ((int (*)(FILE *, const char *, va_list)) get_func_addr(STDIO_ID, 47))
#define vsprintf ((int (*)(char *, const char *, va_list)) get_func_addr(STDIO_ID, 48))
#define vsnprintf ((int (*)(char *, size_t, const char *, va_list)) get_func_addr(STDIO_ID, 49))
#define vprintf_s ((int (*)(const char *, va_list)) get_func_addr(STDIO_ID, 50))
#define vfprintf_s ((int (*)(FILE *, const char *, va_list)) get_func_addr(STDIO_ID, 51))
#define vsprintf_s ((int (*)(char *, rsize_t, const char *, va_list)) get_func_addr(STDIO_ID, 52))
#define vsnprintf_s ((int (*)(char *, rsize_t, rsize_t, const char *, va_list)) get_func_addr(STDIO_ID, 53))
#define ftell ((long (*)(FILE *)) get_func_addr(STDIO_ID, 54))
#define feof ((int (*)(FILE *)) get_func_addr(STDIO_ID, 55))
#define ferror ((int (*)(FILE *)) get_func_addr(STDIO_ID, 56))
#define perror ((void (*)(const char *)) get_func_addr(STDIO_ID, 57))
#define remove ((int (*)(const char *)) get_func_addr(STDIO_ID, 58))
#define rename ((int (*)(const char *, const char *)) get_func_addr(STDIO_ID, 59))
#define tmpfile ((FILE *(*)(void)) get_func_addr(STDIO_ID, 60))
#define tmpfile_s ((errno_t (*)(FILE **)) get_func_addr(STDIO_ID, 61))
#define tmpnam ((char *(*)(char *)) get_func_addr(STDIO_ID, 62))
#define tmpnam_s ((errno_t (*)(char *, rsize_t)) get_func_addr(STDIO_ID, 63))
#endif

#endif
