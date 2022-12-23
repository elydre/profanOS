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

#define clearerr(stream) ((void (*)(FILE *)) get_func_addr(STDIO_ID, 4))(stream)
#define fopen(filename, mode) ((FILE * (*)(const char *, const char *)) get_func_addr(STDIO_ID, 5))(filename, mode)
#define fopen_s(streamptr, filename, mode) ((errno_t (*)(FILE * *, const char *, const char *)) get_func_addr(STDIO_ID, 6))(streamptr, filename, mode)
#define freopen(filename, mode, stream) ((FILE * (*)(const char *, const char *, FILE *)) get_func_addr(STDIO_ID, 7))(filename, mode, stream)
#define freopen_s(newstreamptr, filename, mode, stream) ((errno_t (*)(FILE * *, const char *, const char *, FILE *)) get_func_addr(STDIO_ID, 8))(newstreamptr, filename, mode, stream)
#define fclose(stream) ((int (*)(FILE *)) get_func_addr(STDIO_ID, 9))(stream)
#define fflush(stream) ((int (*)(FILE *)) get_func_addr(STDIO_ID, 10))(stream)
#define setbuf(stream, buffer) ((void (*)(FILE *, char *)) get_func_addr(STDIO_ID, 11))(stream, buffer)
#define setvbuf(stream, buffer, mode, size) ((int (*)(FILE *, char *, int, size_t)) get_func_addr(STDIO_ID, 12))(stream, buffer, mode, size)
#define fwide(stream, mode) ((int (*)(FILE *, int)) get_func_addr(STDIO_ID, 13))(stream, mode)
#define fread(ptr, size, count, stream) ((size_t (*)(void *, size_t, size_t, FILE *)) get_func_addr(STDIO_ID, 14))(ptr, size, count, stream)
#define fwrite(ptr, size, count, stream) ((size_t (*)(const void *, size_t, size_t, FILE *)) get_func_addr(STDIO_ID, 15))(ptr, size, count, stream)
#define fgetc(stream) ((int (*)(FILE *)) get_func_addr(STDIO_ID, 16))(stream)
#define getc(stream) ((int (*)(FILE *)) get_func_addr(STDIO_ID, 17))(stream)
#define fgets(str, num, stream) ((char * (*)(char *, int, FILE *)) get_func_addr(STDIO_ID, 18))(str, num, stream)
#define fputc(c, stream) ((int (*)(int, FILE *)) get_func_addr(STDIO_ID, 19))(c, stream)
#define putc(c, stream) ((int (*)(int, FILE *)) get_func_addr(STDIO_ID, 20))(c, stream)
#define fputs(str, stream) ((int (*)(const char *, FILE *)) get_func_addr(STDIO_ID, 21))(str, stream)
#define getchar() ((int (*)(void)) get_func_addr(STDIO_ID, 22))()
#define gets_s(str, num) ((errno_t (*)(char *, rsize_t)) get_func_addr(STDIO_ID, 23))(str, num)
#define putchar(c) ((int (*)(int)) get_func_addr(STDIO_ID, 24))(c)
#define puts(str) ((int (*)(const char *)) get_func_addr(STDIO_ID, 25))(str)
#define ungetc(c, stream) ((int (*)(int, FILE *)) get_func_addr(STDIO_ID, 26))(c, stream)
#define scanf(format, ...) ((int (*)(const char *, ...)) get_func_addr(STDIO_ID, 27))(format, __VA_ARGS__)
#define fscanf(stream, format, ...) ((int (*)(FILE *, const char *, ...)) get_func_addr(STDIO_ID, 28))(stream, format, __VA_ARGS__)
#define sscanf(str, format, ...) ((int (*)(const char *, const char *, ...)) get_func_addr(STDIO_ID, 29))(str, format, __VA_ARGS__)
#define scanf_s(format, ...) ((int (*)(const char *, ...)) get_func_addr(STDIO_ID, 30))(format, __VA_ARGS__)
#define fscanf_s(stream, format, ...) ((int (*)(FILE *, const char *, ...)) get_func_addr(STDIO_ID, 31))(stream, format, __VA_ARGS__)
#define sscanf_s(str, format, ...) ((int (*)(const char *, const char *, ...)) get_func_addr(STDIO_ID, 32))(str, format, __VA_ARGS__)
#define vscanf(format, arg) ((int (*)(const char *, va_list)) get_func_addr(STDIO_ID, 33))(format, arg)
#define vfscanf(stream, format, arg) ((int (*)(FILE *, const char *, va_list)) get_func_addr(STDIO_ID, 34))(stream, format, arg)
#define vsscanf(str, format, arg) ((int (*)(const char *, const char *, va_list)) get_func_addr(STDIO_ID, 35))(str, format, arg)
#define vscanf_s(format, arg) ((int (*)(const char *, va_list)) get_func_addr(STDIO_ID, 36))(format, arg)
#define vfscanf_s(stream, format, arg) ((int (*)(FILE *, const char *, va_list)) get_func_addr(STDIO_ID, 37))(stream, format, arg)
#define vsscanf_s(str, format, arg) ((int (*)(const char *, const char *, va_list)) get_func_addr(STDIO_ID, 38))(str, format, arg)
#define printf(format, ...) ((int (*)(const char *, ...)) get_func_addr(STDIO_ID, 39))(format, __VA_ARGS__)
#define fprintf(stream, format, ...) ((int (*)(FILE *, const char *, ...)) get_func_addr(STDIO_ID, 40))(stream, format, __VA_ARGS__)
#define sprintf(str, format, ...) ((int (*)(char *, const char *, ...)) get_func_addr(STDIO_ID, 41))(str, format, __VA_ARGS__)
#define snprintf(str, size, format, ...) ((int (*)(char *, size_t, const char *, ...)) get_func_addr(STDIO_ID, 42))(str, size, format, __VA_ARGS__)
#define printf_s(format, ...) ((int (*)(const char *, ...)) get_func_addr(STDIO_ID, 43))(format, __VA_ARGS__)
#define fprintf_s(stream, format, ...) ((int (*)(FILE *, const char *, ...)) get_func_addr(STDIO_ID, 44))(stream, format, __VA_ARGS__)
#define sprintf_s(str, size, format, ...) ((int (*)(char *, rsize_t, const char *, ...)) get_func_addr(STDIO_ID, 45))(str, size, format, __VA_ARGS__)
#define snprintf_s(str, size, count, format, ...) ((int (*)(char *, rsize_t, rsize_t, const char *, ...)) get_func_addr(STDIO_ID, 46))(str, size, count, format, __VA_ARGS__)
#define vprintf(format, arg) ((int (*)(const char *, va_list)) get_func_addr(STDIO_ID, 47))(format, arg)
#define vfprintf(stream, format, arg) ((int (*)(FILE *, const char *, va_list)) get_func_addr(STDIO_ID, 48))(stream, format, arg)
#define vsprintf(str, format, arg) ((int (*)(char *, const char *, va_list)) get_func_addr(STDIO_ID, 49))(str, format, arg)
#define vsnprintf(str, size, format, arg) ((int (*)(char *, size_t, const char *, va_list)) get_func_addr(STDIO_ID, 50))(str, size, format, arg)
#define vprintf_s(format, arg) ((int (*)(const char *, va_list)) get_func_addr(STDIO_ID, 51))(format, arg)
#define vfprintf_s(stream, format, arg) ((int (*)(FILE *, const char *, va_list)) get_func_addr(STDIO_ID, 52))(stream, format, arg)
#define vsprintf_s(str, size, format, arg) ((int (*)(char *, rsize_t, const char *, va_list)) get_func_addr(STDIO_ID, 53))(str, size, format, arg)
#define vsnprintf_s(str, size, count, format, arg) ((int (*)(char *, rsize_t, rsize_t, const char *, va_list)) get_func_addr(STDIO_ID, 54))(str, size, count, format, arg)
#define ftell(stream) ((long (*)(FILE *)) get_func_addr(STDIO_ID, 55))(stream)
#define feof(stream) ((int (*)(FILE *)) get_func_addr(STDIO_ID, 56))(stream)
#define ferror(stream) ((int (*)(FILE *)) get_func_addr(STDIO_ID, 57))(stream)
#define perror(str) ((void (*)(const char *)) get_func_addr(STDIO_ID, 58))(str)
#define remove(filename) ((int (*)(const char *)) get_func_addr(STDIO_ID, 59))(filename)
#define rename(oldname, newname) ((int (*)(const char *, const char *)) get_func_addr(STDIO_ID, 60))(oldname, newname)
#define tmpfile() ((FILE *(*)(void)) get_func_addr(STDIO_ID, 61))()
#define tmpfile_s(stream) ((errno_t (*)(FILE **)) get_func_addr(STDIO_ID, 62))(stream)
#define tmpnam(str) ((char *(*)(char *)) get_func_addr(STDIO_ID, 63))(str)
#define tmpnam_s(str, size) ((errno_t (*)(char *, rsize_t)) get_func_addr(STDIO_ID, 64))(str, size)

#endif
