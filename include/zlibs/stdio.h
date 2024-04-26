/*****************************************************************************\
|   === stdio.h : 2024 ===                                                    |
|                                                                             |
|    Implementation of the stdio.h header file from libC           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef STDIO_H
#define STDIO_H

#include <profan/type.h>
#include <stdarg.h>
#include <stddef.h>

#define stdin  (FILE *) 3
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
#define BUFSIZ 1024

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif
#ifndef SEEK_SET
#define SEEK_SET 0
#endif

void clearerr(FILE *stream);
FILE *fopen(const char *filename, const char *mode);
errno_t fopen_s(FILE **streamptr, const char *filename, const char *mode);
FILE *freopen(const char *filename, const char *mode, FILE *stream);
errno_t freopen_s(FILE **newstreamptr, const char *filename, const char *mode, FILE *stream);
int fclose(FILE *stream);
int fflush(FILE *stream);
void setbuf(FILE *stream, char *buffer);
int setvbuf(FILE *stream, char *buffer, int mode, size_t size);
int fwide(FILE *stream, int mode);
size_t fread(void *buffer, size_t size, size_t count, FILE *stream);
size_t fwrite(const void *buffer, size_t size, size_t count, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
int fgetc(FILE *stream);
int getc(FILE *stream);
char *fgets(char *str, int count, FILE *stream);
int fputc(int ch, FILE *stream);
int putc(int ch, FILE *stream);
int fputs(const char *str, FILE *stream);
int getchar(void);
char *gets_s(char *str, rsize_t n);
int putchar(int ch);
int puts(const char *str);
ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
int ungetc(int ch, FILE *stream);
int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int sscanf(const char *buffer, const char *format, ...);
int scanf_s(const char *format, ...);
int fscanf_s(FILE *stream, const char *format, ...);
int sscanf_s(const char *buffer, const char *format, ...);
int vscanf(const char *format, va_list vlist);
int vfscanf(FILE *stream, const char *format, va_list vlist);
int vsscanf(const char *buffer, const char *format, va_list vlist);
int vscanf_s(const char *format, va_list vlist);
int vfscanf_s(FILE *stream, const char *format, va_list vlist);
int vsscanf_s(const char *buffer, const char *format, va_list vlist);
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *buffer, const char *format, ...);
int snprintf(char* str, size_t size, const char* format, ...);
int printf_s(const char *format, ...);
int fprintf_s(FILE *stream, const char *format, ...);
int sprintf_s(char *buffer, rsize_t bufsz, const char *format, ...);
int snprintf_s(char *buffer, rsize_t bufsz, const char *format, ...);
int vprintf(const char *format, va_list vlist);
int vfprintf(FILE *stream, const char *format, va_list vlist);
int vsprintf(char *buffer, const char *format, va_list vlist);
int vsnprintf(char *str, size_t count, const char *fmt, va_list args);
int vprintf_s(const char *format, va_list vlist);
int vfprintf_s(FILE *stream, const char *format, va_list vlist);
int vsprintf_s(char *buffer, rsize_t bufsz, const char *format, va_list vlist);
int vsnprintf_s(char *buffer, rsize_t bufsz, const char *format, va_list vlist);
long ftell(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
void perror(const char *s);
int remove(const char *fname);
int rename(const char *old_filename, const char *new_filename);
FILE *tmpfile(void);
errno_t tmpfile_s(FILE **streamptr);
char *tmpnam(char *filename);
errno_t tmpnam_s(char *filename_s, rsize_t maxsize);

#endif
