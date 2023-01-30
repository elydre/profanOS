#include <syscall.h>
#include <i_iolib.h>
#include <i_string.h>
#include <type.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#define STDIO_C
#include <stdio.h>
#include <ctype.h>

#define DP_S_DEFAULT 0
#define DP_S_FLAGS   1
#define DP_S_MIN     2
#define DP_S_DOT     3
#define DP_S_MAX     4
#define DP_S_MOD     5
#define DP_S_CONV    6
#define DP_S_DONE    7

/* format flags - Bits */
#define DP_F_MINUS      (1 << 0)
#define DP_F_PLUS       (1 << 1)
#define DP_F_SPACE      (1 << 2)
#define DP_F_NUM        (1 << 3)
#define DP_F_ZERO       (1 << 4)
#define DP_F_UP         (1 << 5)
#define DP_F_UNSIGNED   (1 << 6)

/* Conversion Flags */
#define DP_C_SHORT   1
#define DP_C_LONG    2
#define DP_C_LDOUBLE 3
#define DP_C_LLONG   4

#define char_to_int(p) ((p)- '0')
#ifndef MAX
#define MAX(p,q) (((p) >= (q)) ? (p) : (q))
#endif

void init_func();
int printf(const char *restrict format, ... );
int fflush(FILE *stream);
int fclose(FILE *stream);
static size_t dopr(char *buffer, size_t maxlen, const char *format, 
                   va_list args);
static void fmtstr(char *buffer, size_t *currlen, size_t maxlen,
                    char *value, int flags, int min, int max);
static void fmtint(char *buffer, size_t *currlen, size_t maxlen,
                    long value, int base, int min, int max, int flags);
static void fmtfp(char *buffer, size_t *currlen, size_t maxlen,
                   LDOUBLE fvalue, int min, int max, int flags);
static void dopr_outch(char *buffer, size_t *currlen, size_t maxlen, char c);
int vsnprintf (char *restrict str, size_t count, const char *restrict fmt, va_list args);

int main() {
    init_func();
    return 0;
}

void init_func() {
    c_kprint("Init of the stdio lib !\n");
}

void fonction_inutile() {} // car j'ai tout décalé dans le header, et flemme de fix ça

void clearerr(FILE *stream) {
    fsprint("clearerr not implemented yet, WHY DO YOU USE IT ?\n");
}

FILE *fopen( const char *restrict filename, const char *restrict mode ) {
    // first we check if the file exists with a syscall
    // copy the filename to a new string
    char *file_name = calloc(strlen(filename) + 1, sizeof(char));
    strcpy(file_name, filename);
    int exists = c_fs_does_path_exists(file_name);
    free(file_name);
    // the file doesn't exist but it should
    if (exists == 0 && (strcmp(mode, "r") == 0 || strcmp(mode, "r+") == 0)) {
        return NULL;
    }
    // we separate the filename from the path
    char *name = calloc(strlen(filename) + 1, sizeof(char));
    char *path = calloc(strlen(filename) + 1, sizeof(char));
    // path is everything before the last /
    // filename is everything after the last /
    int last_slash = 0;
    for (unsigned int i = 0; i < strlen(filename); i++) {
        if (filename[i] == '/') {
            last_slash = i;
        }
    }
    for (int i = 0; i < last_slash; i++) {
        path[i] = filename[i];
    }
    for (unsigned int i = last_slash + 1; i < strlen(filename); i++) {
        name[i - last_slash - 1] = filename[i];
    }
    // now we create the file if it doesn't exist
    if (exists == 0) {
        c_fs_make_file(path, name);
    }
    // now the file exists, we can open it
    // we create a new file struct
    FILE *file = calloc(1, sizeof(FILE));
    // we copy the filename, name, path, mode into the file struct
    file->filename = calloc(strlen(filename) + 1, sizeof(char));
    strcpy(file->filename, filename);
    file->name = calloc(strlen(name) + 1, sizeof(char));
    strcpy(file->name, name);
    file->path = calloc(strlen(path) + 1, sizeof(char));
    strcpy(file->path, path);
    file->mode = calloc(strlen(mode) + 1, sizeof(char));
    strcpy(file->mode, mode);
    // we free the name and path, now if we need them we can get them from the file struct
    free(name);
    free(path);

    // we open the file, read it, and put it in the file struct
    // we get the file size
    int file_size = c_fs_get_file_size(file->filename);
    // we allocate the memory for the file
    file->buffer = calloc(file_size + 1, sizeof(char));
    // we read the file
    c_fs_read_file(file->filename, (uint8_t *) file->buffer);
    // we set the file size
    file->buffer_size = file_size;
    // we set the file position to 0
    file->buffer_pos = 0;
    // we set the file eof to 0
    file->eof = 0;
    // we set the file error to 0
    file->error = 0;
    // we set the file temporary to 0
    file->is_temp = 0;
    
    // we return the file
    return file;
}

errno_t fopen_s(FILE *restrict *restrict streamptr, const char *restrict filename, const char *restrict mode ) {
    fsprint("fopen_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

FILE *freopen(const char *restrict filename, const char *restrict mode, FILE *restrict stream) {
    // we close the file
    fclose(stream);
    // we open the file
    return fopen(filename, mode);
}

errno_t freopen_s( FILE *restrict *restrict newstreamptr, const char *restrict filename, const char *restrict mode, FILE *restrict stream ) {
    // we close the file
    fclose(stream);
    // we open the file
    *newstreamptr = fopen(filename, mode);
    return 0;
}

int fclose(FILE *stream) {
    // because we dont actually use streams, we just have to free things

    // but we still have to check if the file isnt null
    if (stream == NULL) {
        return 0;
    }

    // we check if the file is open for writing
    if (strcmp(stream->mode, "w") == 0 || strcmp(stream->mode, "w+") == 0 || strcmp(stream->mode, "a") == 0 || strcmp(stream->mode, "a+") == 0) {
        // we write the file
        fflush(stream);
    }

    free(stream->filename);
    free(stream->path);
    free(stream->name);
    free(stream->mode);
    free(stream->buffer);
    free(stream);
    return 0;
}

int fflush(FILE *stream) {
    // we check if the file is null
    if (stream == NULL) {
        return 0;
    }
    // we check if the file is open for writing
    if (strcmp(stream->mode, "w") != 0 && strcmp(stream->mode, "w+") != 0 && strcmp(stream->mode, "a") != 0 && strcmp(stream->mode, "a+") != 0) {
        return 0;
    }
    // we write the file
    c_fs_write_in_file(stream->filename, (uint8_t *) stream->buffer, stream->buffer_size);
    return 0;
}

void setbuf(FILE *restrict stream, char *restrict buffer) {
    fsprint("setbuf not implemented yet, WHY DO YOU USE IT ?\n");
}

int setvbuf(FILE *restrict stream, char *restrict buffer, int mode, size_t size) {
    fsprint("setvbuf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fwide(FILE *stream, int mode) {
    fsprint("fwide not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

size_t fread(void *restrict buffer, size_t size, size_t count, FILE *restrict stream) {
    // we check if the file is null
    if (stream == NULL) {
        return 0;
    }
    // we check if the file is open for reading
    if (!(strcmp(stream->mode, "r") == 0 || strcmp(stream->mode, "r+") == 0 || strcmp(stream->mode, "w+") == 0 || strcmp(stream->mode, "a+") == 0)) {
        return 0;
    }
    // we check if the file is at the end
    if (stream->eof == 1) {
        return 0;
    }
    // we copy char by char from the file buffer to the buffer
    while (count && !stream->eof) {
        // we check if the file is at the end
        if (stream->buffer_pos >= stream->buffer_size) {
            stream->eof = 1;
            return 0;
        }
        // we copy the char
        ((char *) buffer)[stream->buffer_pos] = stream->buffer[stream->buffer_pos];
        // we increment the buffer position
        stream->buffer_pos++;
        // we decrement the count
        count--;
    }
    return 1;
}

size_t fwrite(const void *restrict buffer, size_t size, size_t count, FILE *restrict stream) {
    // we check if the file is null
    if (stream == NULL) {
        return 0;
    }
    // we check if the file is open for reading
    if (strcmp(stream->mode, "r") == 0 || strcmp(stream->mode, "r+") == 0) {
        return 0;
    }
    // now we branch, it's not the same in append and write mode
    if (strcmp(stream->mode, "w") == 0 || strcmp(stream->mode, "w+") == 0) {
        // we reset the file buffer
        free(stream->buffer);
        stream->buffer = calloc(size * count + 1, sizeof(char));
        // we copy the buffer into the file buffer
        strcpy(stream->buffer, buffer);
        // we set the buffer size
        stream->buffer_size = size * count;
        // we set the buffer position
        stream->buffer_pos = 0;
        // we set the eof
        stream->eof = 0;
        // we set the error
        stream->error = 0;
        // we flush the file
        fflush(stream);
    }
    if (strcmp(stream->mode, "a") == 0 || strcmp(stream->mode, "a+") == 0) {
        // in this mode we just create a string that is the concatenation of the file buffer and the buffer
        char *new_buffer = calloc(stream->buffer_size + size * count + 1, sizeof(char));
        strcpy(new_buffer, stream->buffer);
        strcat(new_buffer, buffer);
        // we free the old buffer
        free(stream->buffer);
        // we set the new buffer
        stream->buffer = new_buffer;
        // we set the buffer size
        stream->buffer_size = stream->buffer_size + size * count;
        // we set the buffer position
        stream->buffer_pos = 0;
        // we set the eof
        stream->eof = 0;
        // we set the error
        stream->error = 0;
        // we flush the file
        fflush(stream);
    }

    // in any case, we return the count
    return count;
}

int fgetc(FILE *stream) {
    fsprint("fgetc not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int getc(FILE *stream) {
    fsprint("getc not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *fgets(char *restrict str, int count, FILE *restrict stream) {
    fsprint("fgets not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

int fputc( int ch, FILE *stream ) {
    fsprint("fputc not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int putc( int ch, FILE *stream ) {
    fsprint("putc not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fputs(const char *restrict str, FILE *restrict stream) {
    fsprint("fputs not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int getchar(void) {
    // Equivalent to getc(stdin). (https://en.cppreference.com/w/c/io/getchar)
    fsprint("getchar not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *gets_s( char *str, rsize_t n ) {
    fsprint("gets_s not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

int putchar(int ch) {
    fsprint("putchar not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int puts(const char *str) {
    fsprint("puts not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int ungetc(int ch, FILE *stream) {
    fsprint("ungetc not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int scanf( const char *restrict format, ... ) {
    fsprint("scanf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fscanf( FILE *restrict stream, const char *restrict format, ... ) {
    fsprint("fscanf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int sscanf( const char *restrict buffer, const char *restrict format, ... ) {
    fsprint("sscanf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int scanf_s(const char *restrict format, ...) {
    fsprint("scanf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fscanf_s(FILE *restrict stream, const char *restrict format, ...) {
    fsprint("fscanf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int sscanf_s(const char *restrict buffer, const char *restrict format, ...) {
    fsprint("sscanf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vscanf( const char *restrict format, va_list vlist ) {
    fsprint("vscanf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vfscanf( FILE *restrict stream, const char *restrict format, va_list vlist ) {
    fsprint("vfscanf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vsscanf( const char *restrict buffer, const char *restrict format, va_list vlist ) {
    fsprint("vsscanf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vscanf_s(const char *restrict format, va_list vlist) {
    fsprint("vscanf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vfscanf_s( FILE *restrict stream, const char *restrict format, va_list vlist) {
    fsprint("vfscanf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vsscanf_s( const char *restrict buffer, const char *restrict format, va_list vlist) {
    fsprint("vsscanf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int printf(const char *restrict format, ...) {
    va_list args;
    // we copy format to a buffer because we need to modify it
    char *format_copy = malloc(strlen(format) + 1);
    strcpy(format_copy, format);
    va_start(args, format);
    vfsprint(format_copy, args);
    va_end(args);
    free(format_copy);
    return 0;
}

int fprintf( FILE *restrict stream, const char *restrict format, ... ) {
    fsprint("fprintf is not correctly implemented, ignore this warning if you are not Loris !\n");
    // if the stream is read only, we can't write to it
    if (strcmp(stream->mode, "r") == 0 || strcmp(stream->mode, "r+") == 0 || stream == stdin) {
        return 0;
    }
    // if the stream is stdout or stderr, we use vfsprint
    if (stream == stdout || stream == stderr) {
        va_list args;
        // we copy format to a buffer because we need to modify it
        char *format_copy = malloc(strlen(format) + 1);
        strcpy(format_copy, format);
        va_start(args, format);
        vfsprint(format_copy, args);
        va_end(args);
        free(format_copy);
        return strlen(format); // TODO : return the true number of characters written
    }
    // if the stream is a file, we show an error, it's not implemented yet
    fsprint("fprintf not implemented for files yet, WHY DO YOU USE IT ?\n");
    return 0; // TODO : return the true number of characters written
}

int sprintf( char *restrict buffer, const char *restrict format, ... ) {
    fsprint("sprintf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

/*
* Copyright Patrick Powell 1995
* This code is based on code written by Patrick Powell (papowell@astart.com)
* It may be used for any purpose as long as this notice remains intact
* on all source code distributions
*/
/**************************************************************
* Original:
* Patrick Powell Tue Apr 11 09:48:21 PDT 1995
* A bombproof version of doprnt (dopr) included.
* Sigh.  This sort of thing is always nasty do deal with.  Note that
* the version here does not include floating point...
*
* snprintf() is used instead of sprintf() as it does limit checks
* for string length.  This covers a nasty loophole.
*
* The other functions are there to prevent NULL pointers from
* causing nast effects.
*
* More Recently:
*  Brandon Long <blong@fiction.net> 9/15/96 for mutt 0.43
*  This was ugly.  It is still ugly.  I opted out of floating point
*  numbers, but the formatter understands just about everything
*  from the normal C string format, at least as far as I can tell from
*  the Solaris 2.5 printf(3S) man page.
*
*  Brandon Long <blong@fiction.net> 10/22/97 for mutt 0.87.1
*    Ok, added some minimal floating point support, which means this
*    probably requires libm on most operating systems.  Don't yet
*    support the exponent (e,E) and sigfig (g,G).  Also, fmtint()
*    was pretty badly broken, it just wasn't being exercised in ways
*    which showed it, so that's been fixed.  Also, formated the code
*    to mutt conventions, and removed dead code left over from the
*    original.  Also, there is now a builtin-test, just compile with:
*           gcc -DTEST_SNPRINTF -o snprintf snprintf.c -lm
*    and run snprintf for results.
* 
*  Thomas Roessler <roessler@guug.de> 01/27/98 for mutt 0.89i
*    The PGP code was using unsigned hexadecimal formats. 
*    Unfortunately, unsigned formats simply didn't work.
*
*  Michael Elkins <me@cs.hmc.edu> 03/05/98 for mutt 0.90.8
*    The original code assumed that both snprintf() and vsnprintf() were
*    missing.  Some systems only have snprintf() but not vsnprintf(), so
*    the code is now broken down under HAVE_SNPRINTF and HAVE_VSNPRINTF.
*
*  Andrew Tridgell (tridge@samba.org) Oct 1998
*    fixed handling of %.0f
*    added test for HAVE_LONG_DOUBLE
*
* tridge@samba.org, idra@samba.org, April 2001
*    got rid of fcvt code (twas buggy and made testing harder)
*    added C99 semantics
*
**************************************************************/
 int snprintf(char *str,size_t count,const char *fmt,...)
{
        size_t ret;
        va_list ap;
    
        va_start(ap, fmt);
        ret = vsnprintf(str, count, fmt, ap);
        va_end(ap);
        return ret;
}

int printf_s( const char *restrict format, ... ) {
    fsprint("printf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fprintf_s( FILE *restrict stream, const char *restrict format, ... ) {
    fsprint("fprintf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int sprintf_s( char *restrict buffer, rsize_t bufsz, const char *restrict format, ... ) {
    fsprint("sprintf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int snprintf_s( char *restrict buffer, rsize_t bufsz, const char *restrict format, ... ) {
    fsprint("snprintf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vprintf( const char *restrict format, va_list vlist ) {
    fsprint("vprintf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vfprintf( FILE *restrict stream, const char *restrict format, va_list vlist ) {
    fsprint("vfprintf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vsprintf( char *restrict buffer, const char *restrict format, va_list vlist ) {
    fsprint("vsprintf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vsnprintf (char *restrict str, size_t count, const char *restrict fmt, va_list args)
{
    return dopr(str, count, fmt, args);
}

int vprintf_s( const char *restrict format, va_list vlist ) {
    fsprint("vprintf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vfprintf_s( FILE *restrict stream, const char *restrict format, va_list vlist ) {
    fsprint("vfprintf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vsprintf_s( char *restrict buffer, rsize_t bufsz, const char *restrict format, va_list vlist ) {
    fsprint("vsprintf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vsnprintf_s( char *restrict buffer, rsize_t bufsz, const char *restrict format, va_list vlist ) {
    fsprint("vsnprintf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

long ftell( FILE *stream ) {
    fsprint("ftell not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int feof( FILE *stream ) {
    fsprint("feof not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int ferror( FILE *stream ) {
    fsprint("ferror not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

void perror( const char *s ) {
    fsprint("perror not implemented yet, WHY DO YOU USE IT ?\n");
}

int remove( const char *fname ) {
    fsprint("remove not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int rename( const char *old_filename, const char *new_filename ) {
    fsprint("rename not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

// TODO : IMPLEMENT RANDOM BEFORE THIS
FILE *tmpfile(void) {
    fsprint("tmpfile not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

// TODO : IMPLEMENT RANDOM BEFORE THIS
errno_t tmpfile_s(FILE * restrict * restrict streamptr) {
    fsprint("tmpfile_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *tmpnam( char *filename ) {
    fsprint("tmpnam not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

errno_t tmpnam_s(char *filename_s, rsize_t maxsize) {
    fsprint("tmpnam_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

static size_t dopr(char *buffer, size_t maxlen, const char *format, va_list args) {
    char ch;
    LLONG value;
    LDOUBLE fvalue;
    char *strvalue;
    int min;
    int max;
    int state;
    int flags;
    int cflags;
    size_t currlen;
    
    state = DP_S_DEFAULT;
    currlen = flags = cflags = min = 0;
    max = -1;
    ch = *format++;
    
    while (state != DP_S_DONE) {
            if (ch == '\0') 
                    state = DP_S_DONE;

            switch(state) {
            case DP_S_DEFAULT:
                    if (ch == '%') 
                            state = DP_S_FLAGS;
                    else 
                            dopr_outch (buffer, &currlen, maxlen, ch);
                    ch = *format++;
                    break;
            case DP_S_FLAGS:
                    switch (ch) {
                    case '-':
                            flags |= DP_F_MINUS;
                            ch = *format++;
                            break;
                    case '+':
                            flags |= DP_F_PLUS;
                            ch = *format++;
                            break;
                    case ' ':
                            flags |= DP_F_SPACE;
                            ch = *format++;
                            break;
                    case '#':
                            flags |= DP_F_NUM;
                            ch = *format++;
                            break;
                    case '0':
                            flags |= DP_F_ZERO;
                            ch = *format++;
                            break;
                    default:
                            state = DP_S_MIN;
                            break;
                    }
                    break;
            case DP_S_MIN:
                    if (isdigit((unsigned char)ch)) {
                            min = 10*min + char_to_int (ch);
                            ch = *format++;
                    } else if (ch == '*') {
                            min = va_arg (args, int);
                            ch = *format++;
                            state = DP_S_DOT;
                    } else {
                            state = DP_S_DOT;
                    }
                    break;
            case DP_S_DOT:
                    if (ch == '.') {
                            state = DP_S_MAX;
                            ch = *format++;
                    } else { 
                            state = DP_S_MOD;
                    }
                    break;
            case DP_S_MAX:
                    if (isdigit((unsigned char)ch)) {
                            if (max < 0)
                                    max = 0;
                            max = 10*max + char_to_int (ch);
                            ch = *format++;
                    } else if (ch == '*') {
                            max = va_arg (args, int);
                            ch = *format++;
                            state = DP_S_MOD;
                    } else {
                            state = DP_S_MOD;
                    }
                    break;
            case DP_S_MOD:
                    switch (ch) {
                    case 'h':
                            cflags = DP_C_SHORT;
                            ch = *format++;
                            break;
                    case 'l':
                            cflags = DP_C_LONG;
                            ch = *format++;
                            if (ch == 'l') {        /* It's a long long */
                                    cflags = DP_C_LLONG;
                                    ch = *format++;
                            }
                            break;
                    case 'L':
                            cflags = DP_C_LDOUBLE;
                            ch = *format++;
                            break;
                    default:
                            break;
                    }
                    state = DP_S_CONV;
                    break;
            case DP_S_CONV:
                    switch (ch) {
                    case 'd':
                        [[fallthrough]];
                    case 'i':
                            if (cflags == DP_C_SHORT) 
                                    value = va_arg (args, int);
                            else if (cflags == DP_C_LONG)
                                    value = va_arg (args, long int);
                            else if (cflags == DP_C_LLONG)
                                    value = va_arg (args, LLONG);
                            else
                                    value = va_arg (args, int);
                            fmtint (buffer, &currlen, maxlen, value, 10, min, max, flags);
                            break;
                    case 'o':
                            flags |= DP_F_UNSIGNED;
                            if (cflags == DP_C_SHORT)
                                    value = va_arg (args, unsigned int);
                            else if (cflags == DP_C_LONG)
                                    value = (long)va_arg (args, unsigned long int);
                            else if (cflags == DP_C_LLONG)
                                    value = (long)va_arg (args, unsigned LLONG);
                            else
                                    value = (long)va_arg (args, unsigned int);
                            fmtint (buffer, &currlen, maxlen, value, 8, min, max, flags);
                            break;
                    case 'u':
                            flags |= DP_F_UNSIGNED;
                            if (cflags == DP_C_SHORT)
                                    value = va_arg (args, unsigned int);
                            else if (cflags == DP_C_LONG)
                                    value = (long)va_arg (args, unsigned long int);
                            else if (cflags == DP_C_LLONG)
                                    value = (LLONG)va_arg (args, unsigned LLONG);
                            else
                                    value = (long)va_arg (args, unsigned int);
                            fmtint (buffer, &currlen, maxlen, value, 10, min, max, flags);
                            break;
                    case 'X':
                            flags |= DP_F_UP;
                            [[fallthrough]];
                    case 'x':
                            flags |= DP_F_UNSIGNED;
                            if (cflags == DP_C_SHORT)
                                    value = va_arg (args, unsigned int);
                            else if (cflags == DP_C_LONG)
                                    value = (long)va_arg (args, unsigned long int);
                            else if (cflags == DP_C_LLONG)
                                    value = (LLONG)va_arg (args, unsigned LLONG);
                            else
                                    value = (long)va_arg (args, unsigned int);
                            fmtint (buffer, &currlen, maxlen, value, 16, min, max, flags);
                            break;
                    case 'f':
                            if (cflags == DP_C_LDOUBLE)
                                    fvalue = va_arg (args, LDOUBLE);
                            else
                                    fvalue = va_arg (args, double);
                            /* um, floating point? */
                            fmtfp (buffer, &currlen, maxlen, fvalue, min, max, flags);
                            break;
                    case 'E':
                            flags |= DP_F_UP;
                            [[fallthrough]];
                    case 'e':
                            if (cflags == DP_C_LDOUBLE)
                                    fvalue = va_arg (args, LDOUBLE);
                            else
                                    fvalue = va_arg (args, double);
                            break;
                    case 'G':
                            flags |= DP_F_UP;
                            [[fallthrough]];
                    case 'g':
                            if (cflags == DP_C_LDOUBLE)
                                    fvalue = va_arg (args, LDOUBLE);
                            else
                                    fvalue = va_arg (args, double);
                            break;
                    case 'c':
                            dopr_outch (buffer, &currlen, maxlen, va_arg (args, int));
                            break;
                    case 's':
                            strvalue = va_arg (args, char *);
                            if (max == -1) {
                                    max = strlen(strvalue);
                            }
                            if (min > 0 && max >= 0 && min > max) max = min;
                            fmtstr (buffer, &currlen, maxlen, strvalue, flags, min, max);
                            break;
                    case 'p':
                            strvalue = va_arg (args, void *);
                            fmtint (buffer, &currlen, maxlen, (long) strvalue, 16, min, max, flags);
                            break;
                    case 'n':
                            if (cflags == DP_C_SHORT) {
                                    short int *num;
                                    num = va_arg (args, short int *);
                                    *num = currlen;
                            } else if (cflags == DP_C_LONG) {
                                    long int *num;
                                    num = va_arg (args, long int *);
                                    *num = (long int)currlen;
                            } else if (cflags == DP_C_LLONG) {
                                    LLONG *num;
                                    num = va_arg (args, LLONG *);
                                    *num = (LLONG)currlen;
                            } else {
                                    int *num;
                                    num = va_arg (args, int *);
                                    *num = currlen;
                            }
                            break;
                    case '%':
                            dopr_outch (buffer, &currlen, maxlen, ch);
                            break;
                    case 'w':
                            /* not supported yet, treat as next char */
                            ch = *format++;
                            break;
                    default:
                            /* Unknown, skip */
                            break;
                    }
                    ch = *format++;
                    state = DP_S_DEFAULT;
                    flags = cflags = min = 0;
                    max = -1;
                    break;
            case DP_S_DONE:
                    break;
            default:
                    /* hmm? */
                    break; /* some picky compilers need this */
            }
    }
    if (maxlen != 0) {
            if (currlen < maxlen - 1) 
                    buffer[currlen] = '\0';
            else if (maxlen > 0) 
                    buffer[maxlen - 1] = '\0';
    }
    
    return currlen;
}

static void fmtstr(char *buffer, size_t *currlen, size_t maxlen,
                    char *value, int flags, int min, int max)
{
    int padlen, strln;     /* amount to pad */
    int cnt = 0;

#ifdef DEBUG_SNPRINTF
    printf("fmtstr min=%d max=%d s=[%s]\n", min, max, value);
#endif
    if (value == 0) {
            value = "<NULL>";
    }

    for (strln = 0; value[strln]; ++strln); /* strlen */
    padlen = min - strln;
    if (padlen < 0) 
            padlen = 0;
    if (flags & DP_F_MINUS) 
            padlen = -padlen; /* Left Justify */
    
    while ((padlen > 0) && (cnt < max)) {
            dopr_outch (buffer, currlen, maxlen, ' ');
            --padlen;
            ++cnt;
    }
    while (*value && (cnt < max)) {
            dopr_outch (buffer, currlen, maxlen, *value++);
            ++cnt;
    }
    while ((padlen < 0) && (cnt < max)) {
            dopr_outch (buffer, currlen, maxlen, ' ');
            ++padlen;
            ++cnt;
    }
}

static void fmtint(char *buffer, size_t *currlen, size_t maxlen,
                    long value, int base, int min, int max, int flags)
{
    int signvalue = 0;
    unsigned long uvalue;
    char convert[20];
    int place = 0;
    int spadlen = 0; /* amount to space pad */
    int zpadlen = 0; /* amount to zero pad */
    int caps = 0;
    
    if (max < 0)
            max = 0;
    
    uvalue = value;
    
    if(!(flags & DP_F_UNSIGNED)) {
            if( value < 0 ) {
                    signvalue = '-';
                    uvalue = -value;
            } else {
                    if (flags & DP_F_PLUS)  /* Do a sign (+/i) */
                            signvalue = '+';
                    else if (flags & DP_F_SPACE)
                            signvalue = ' ';
            }
    }

    if (flags & DP_F_UP) caps = 1; /* Should characters be upper case? */

    do {
            convert[place++] =
                    (caps? "0123456789ABCDEF":"0123456789abcdef")
                    [uvalue % (unsigned)base  ];
            uvalue = (uvalue / (unsigned)base );
    } while(uvalue && (place < 20));
    if (place == 20) place--;
    convert[place] = 0;

    zpadlen = max - place;
    spadlen = min - MAX (max, place) - (signvalue ? 1 : 0);
    if (zpadlen < 0) zpadlen = 0;
    if (spadlen < 0) spadlen = 0;
    if (flags & DP_F_ZERO) {
            zpadlen = MAX(zpadlen, spadlen);
            spadlen = 0;
    }
    if (flags & DP_F_MINUS) 
            spadlen = -spadlen; /* Left Justifty */

#ifdef DEBUG_SNPRINTF
    printf("zpad: %d, spad: %d, min: %d, max: %d, place: %d\n",
            zpadlen, spadlen, min, max, place);
#endif

    /* Spaces */
    while (spadlen > 0) {
            dopr_outch (buffer, currlen, maxlen, ' ');
            --spadlen;
    }

    /* Sign */
    if (signvalue) 
            dopr_outch (buffer, currlen, maxlen, signvalue);

    /* Zeros */
    if (zpadlen > 0) {
            while (zpadlen > 0) {
                    dopr_outch (buffer, currlen, maxlen, '0');
                    --zpadlen;
            }
    }

    /* Digits */
    while (place > 0) 
            dopr_outch (buffer, currlen, maxlen, convert[--place]);

    /* Left Justified spaces */
    while (spadlen < 0) {
            dopr_outch (buffer, currlen, maxlen, ' ');
            ++spadlen;
    }
}

static LDOUBLE abs_val(LDOUBLE value)
{
    LDOUBLE result = value;

    if (value < 0)
            result = -value;
    
    return result;
}

static LDOUBLE POW10(int exp)
{
    LDOUBLE result = 1;
    
    while (exp) {
            result *= 10;
            exp--;
    }

    return result;
}

static LLONG ROUND(LDOUBLE value)
{
    LLONG intpart;

    intpart = (LLONG)value;
    value = value - intpart;
    if (value >= 0.5) intpart++;
    
    return intpart;
}

/* a replacement for modf that doesn't need the math library. Should
   be portable, but slow */
static double my_modf(double x0, double *iptr)
{
    int i;
    long l;
    double x = x0;
    double f = 1.0;

    for (i=0;i<100;i++) {
            l = (long)x;
            if (l <= (x+1) && l >= (x-1)) break;
            x *= 0.1;
            f *= 10.0;
    }

    if (i == 100) {
            /* yikes! the number is beyond what we can handle. What do we do? */
            (*iptr) = 0;
            return 0;
    }

    if (i != 0) {
            double i2;
            double ret;

            ret = my_modf(x0-l*f, &i2);
            (*iptr) = l*f + i2;
            return ret;
    } 

    (*iptr) = l;
    return x - (*iptr);
}

static void fmtfp (char *buffer, size_t *currlen, size_t maxlen,
                   LDOUBLE fvalue, int min, int max, int flags)
{
    int signvalue = 0;
    double ufvalue;
    char iconvert[311];
    char fconvert[311];
    int iplace = 0;
    int fplace = 0;
    int padlen = 0; /* amount to pad */
    int zpadlen = 0; 
    int caps = 0;
    int index;
    double intpart;
    double fracpart;
    double temp;

    /* 
        * AIX manpage says the default is 0, but Solaris says the default
        * is 6, and sprintf on AIX defaults to 6
        */
    if (max < 0)
            max = 6;

    ufvalue = abs_val (fvalue);

    if (fvalue < 0) {
            signvalue = '-';
    } else {
            if (flags & DP_F_PLUS) { /* Do a sign (+/i) */
                    signvalue = '+';
            } else {
                    if (flags & DP_F_SPACE)
                            signvalue = ' ';
            }
    }

#if 0
    if (flags & DP_F_UP) caps = 1; /* Should characters be upper case? */
#endif

#if 0
        if (max == 0) ufvalue += 0.5; /* if max = 0 we must round */
#endif

    /* 
        * Sorry, we only support 16 digits past the decimal because of our 
        * conversion method
        */
    if (max > 16)
            max = 16;

    /* We "cheat" by converting the fractional part to integer by
        * multiplying by a factor of 10
        */

    temp = ufvalue;
    my_modf(temp, &intpart);

    fracpart = ROUND((POW10(max)) * (ufvalue - intpart));
    
    if (fracpart >= POW10(max)) {
            intpart++;
            fracpart -= POW10(max);
    }


    /* Convert integer part */
    do {
            temp = intpart;
            my_modf(intpart*0.1, &intpart);
            temp = temp*0.1;
            index = (int) ((temp -intpart +0.05)* 10.0);
            /* index = (int) (((double)(temp*0.1) -intpart +0.05) *10.0); */
            /* printf ("%llf, %f, %x\n", temp, intpart, index); */
            iconvert[iplace++] =
                    (caps? "0123456789ABCDEF":"0123456789abcdef")[index];
    } while (intpart && (iplace < 311));
    if (iplace == 311) iplace--;
    iconvert[iplace] = 0;

    /* Convert fractional part */
    if (fracpart)
    {
            do {
                    temp = fracpart;
                    my_modf(fracpart*0.1, &fracpart);
                    temp = temp*0.1;
                    index = (int) ((temp -fracpart +0.05)* 10.0);
                    /* index = (int) ((((temp/10) -fracpart) +0.05) *10); */
                    /* printf ("%lf, %lf, %ld\n", temp, fracpart, index); */
                    fconvert[fplace++] =
                    (caps? "0123456789ABCDEF":"0123456789abcdef")[index];
            } while(fracpart && (fplace < 311));
            if (fplace == 311) fplace--;
    }
    fconvert[fplace] = 0;

    /* -1 for decimal point, another -1 if we are printing a sign */
    padlen = min - iplace - max - 1 - ((signvalue) ? 1 : 0); 
    zpadlen = max - fplace;
    if (zpadlen < 0) zpadlen = 0;
    if (padlen < 0) 
            padlen = 0;
    if (flags & DP_F_MINUS) 
            padlen = -padlen; /* Left Justifty */
    
    if ((flags & DP_F_ZERO) && (padlen > 0)) {
            if (signvalue) {
                    dopr_outch (buffer, currlen, maxlen, signvalue);
                    --padlen;
                    signvalue = 0;
            }
            while (padlen > 0) {
                    dopr_outch (buffer, currlen, maxlen, '0');
                    --padlen;
            }
    }
    while (padlen > 0) {
            dopr_outch (buffer, currlen, maxlen, ' ');
            --padlen;
    }
    if (signvalue) 
            dopr_outch (buffer, currlen, maxlen, signvalue);
    
    while (iplace > 0) 
            dopr_outch (buffer, currlen, maxlen, iconvert[--iplace]);

#ifdef DEBUG_SNPRINTF
    printf("fmtfp: fplace=%d zpadlen=%d\n", fplace, zpadlen);
#endif

    /*
        * Decimal point.  This should probably use locale to find the correct
        * char to print out.
        */
    if (max > 0) {
            dopr_outch (buffer, currlen, maxlen, '.');
            
            while (fplace > 0) 
                    dopr_outch (buffer, currlen, maxlen, fconvert[--fplace]);
    }
    
    while (zpadlen > 0) {
            dopr_outch (buffer, currlen, maxlen, '0');
            --zpadlen;
    }

    while (padlen < 0) {
            dopr_outch (buffer, currlen, maxlen, ' ');
            ++padlen;
    }
}

static void dopr_outch(char *buffer, size_t *currlen, size_t maxlen, char c)
{
    if (*currlen < maxlen) {
            buffer[(*currlen)] = c;
    }
    (*currlen)++;
}

#ifndef HAVE_VASPRINTF
 int vasprintf(char **ptr, const char *format, va_list ap)
{
        int ret;
        
        ret = vsnprintf(NULL, 0, format, ap);
        if (ret <= 0) return ret;

        (*ptr) = (char *)malloc(ret+1);
        if (!*ptr) return -1;
        ret = vsnprintf(*ptr, ret+1, format, ap);

        return ret;
}
#endif

#ifndef HAVE_ASPRINTF
 int asprintf(char **ptr, const char *format, ...)
{
    va_list ap;
    int ret;
    
    va_start(ap, format);
    ret = vasprintf(ptr, format, ap);
    va_end(ap);

    return ret;
}
#endif


