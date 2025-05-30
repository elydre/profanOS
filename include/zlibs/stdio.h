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

#ifndef _STDIO_H
#define _STDIO_H

#include <profan/minimal.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stddef.h>

_BEGIN_C_FILE

// opaque type for FILE
struct _IO_FILE;
typedef struct _IO_FILE FILE;

typedef long fpos_t;

// standard streams
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

// make old C revisions happy
#define stdin stdin
#define stdout stdout
#define stderr stderr

#define EOF -1
#define FOPEN_MAX 1024
#define FILENAME_MAX 20
#define BUFSIZ 1024

#define L_tmpnam 20

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

#undef  SEEK_CUR
#define SEEK_CUR 1

#undef  SEEK_END
#define SEEK_END 2

#undef  SEEK_SET
#define SEEK_SET 0

void    clearerr(FILE *stream);
int     fileno(FILE *stream);
FILE   *fopen(const char *filename, const char *mode);
FILE   *fdopen(int fd, const char *mode);
FILE   *freopen(const char *filename, const char *mode, FILE *stream);
int     fclose(FILE *stream);
int     fflush(FILE *stream);
void    setbuf(FILE *stream, char *buffer);
int     setvbuf(FILE *stream, char *buffer, int mode, size_t size);
int     fwide(FILE *stream, int mode);
size_t  fread(void *buffer, size_t size, size_t count, FILE *stream);
size_t  fwrite(const void *buffer, size_t size, size_t count, FILE *stream);
int     fseek(FILE *stream, long offset, int whence);
long    ftell(FILE *stream);
int     fgetpos(FILE *stream, fpos_t *pos);
int     fsetpos(FILE *stream, const fpos_t *pos);
int     fgetc(FILE *stream);
int     getc(FILE *stream);
char   *fgets(char *str, int count, FILE *stream);
int     fputc(int ch, FILE *stream);
int     putc(int ch, FILE *stream);
int     fputs(const char *str, FILE *stream);
int     getchar(void);
int     putchar(int ch);
int     puts(const char *str);
ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
int     ungetc(int ch, FILE *stream);
int     scanf(const char *format, ...);
int     fscanf(FILE *stream, const char *format, ...);
int     sscanf(const char *buffer, const char *format, ...);
int     vscanf(const char *format, va_list vlist);
int     vfscanf(FILE *stream, const char *format, va_list vlist);
int     vsscanf(const char *buffer, const char *format, va_list vlist);
int     printf(const char *format, ...);
int     fprintf(FILE *stream, const char *format, ...);
int     sprintf(char *buffer, const char *format, ...);
int     snprintf(char* str, size_t size, const char* format, ...);
int     vprintf(const char *format, va_list vlist);
int     vfprintf(FILE *stream, const char *format, va_list vlist);
int     vsprintf(char *buffer, const char *format, va_list vlist);
int     vsnprintf(char *str, size_t count, const char *fmt, va_list args);
int     feof(FILE *stream);
int     ferror(FILE *stream);
void    rewind(FILE *stream);
void    perror(const char *s);
int     remove(const char *fname);
int     rename(const char *old_filename, const char *new_filename);
FILE   *tmpfile(void);
char   *tmpnam(char *filename);

_END_C_FILE

#endif
