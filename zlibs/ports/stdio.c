#include <i_iolib.h>

#include <syscall.h>
#include <stdlib.h>
#include <string.h>
#include <type.h>

#include <stdarg.h>

#define STDIO_C
#include <stdio.h>


void init_func();

int puts(const char *str);

int vprintf(const char *restrict format, va_list vlist);
int dopr(char* str, size_t size, const char* format, va_list arg);

int main() {
    init_func();
    return 0;
}

void init_func() {
    puts("Init of the stdio lib !\n");
}

void clearerr(FILE *stream) {
    puts("clearerr not implemented yet, WHY DO YOU USE IT ?\n");
}

int fflush(FILE *stream);
int fclose(FILE *stream);

FILE *fopen(const char *restrict filename, const char *restrict mode) {
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

    // if we are in writing mode we void the content
    if (strcmp(mode, "w") == 0 || strcmp(mode, "w+") == 0 || strcmp(mode, "wb") == 0 || strcmp(mode, "wb+") == 0) {
        file->buffer[0] = '\0';
        file->buffer_size = 0;
        file->buffer_pos = 0;
    }

    // we return the file
    return file;
}

errno_t fopen_s(FILE *restrict *restrict streamptr, const char *restrict filename, const char *restrict mode) {
    puts("fopen_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

FILE *freopen(const char *restrict filename, const char *restrict mode, FILE *restrict stream) {
    // we close the file
    fclose(stream);
    // we open the file
    return fopen(filename, mode);
}

errno_t freopen_s(FILE *restrict *restrict newstreamptr, const char *restrict filename, const char *restrict mode, FILE *restrict stream) {
    // we close the file
    fclose(stream);
    // we open the file
    *newstreamptr = fopen(filename, mode);
    return 0;
}

int fclose(FILE *stream) {
    // because we dont actually use streams, we just have to free things

    // but we still have to check if the file isnt null
    if (stream == NULL || stream == stdout) {
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
    if (stream == NULL || stream == stdout) {
        return 0;
    }
    // we check if the file is open for writing
    if (strcmp(stream->mode, "w") && strcmp(stream->mode, "w+") && strcmp(stream->mode, "a") && strcmp(stream->mode, "a+") && strcmp(stream->mode, "wb") && strcmp(stream->mode, "wb+") && strcmp(stream->mode, "ab") && strcmp(stream->mode, "ab+")) {
        return 0;
    }
    // we write the file
    c_fs_write_in_file(stream->filename, (uint8_t *) stream->buffer, stream->buffer_pos);
    return 0;
}

void setbuf(FILE *restrict stream, char *restrict buffer) {
    puts("setbuf not implemented yet, WHY DO YOU USE IT ?\n");
}

int setvbuf(FILE *restrict stream, char *restrict buffer, int mode, size_t size) {
    puts("setvbuf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fwide(FILE *stream, int mode) {
    puts("fwide not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

size_t fread(void *restrict buffer, size_t size, size_t count, FILE *restrict stream) {
    // we check if the file is null
    if (stream == NULL || stream == stdout) {
        return 0;
    }
    int mode_len = strlen(stream->mode);
    // we check if the file is open for reading
    for (int i = 0; i < mode_len; i++) {
        if (stream->mode[i] == 'r') {
            break;
        }
        if (i == mode_len - 1) {
            return 0;
        }
    }
    // we check if the file is at the end
    if (stream->eof == 1) {
        return 0;
    }
    // we copy char by char from the file buffer to the buffer
    int read_count = 0;
    while (count && !stream->eof) {
        // we check if the file is at the end
        if (stream->buffer_pos >= stream->buffer_size) {
            stream->eof = 1;
            break;
        }
        // we copy the char
        ((char *) buffer)[read_count] = stream->buffer[stream->buffer_pos];
        // we increment the buffer position
        stream->buffer_pos++;
        // we decrement the count
        count--;
        read_count++;
    }
    return read_count;
}

size_t fwrite(const void *restrict buffer, size_t size, size_t count, FILE *restrict stream) {
    // we check if the file is null
    if (stream == NULL) {
        return 0;
    }

    if (stream == stdout) {
        puts(buffer);
        return count;
    }

    // we check if the file is open for reading
    if (strcmp(stream->mode, "r") == 0 || strcmp(stream->mode, "r+") == 0) {
        return 0;
    }

    // we copy char by char from the buffer to the file buffer
    for (int i = 0; i < (int) count; i++) {
        // we check if the file buffer is full
        if (stream->buffer_pos >= stream->buffer_size) {
            // we realloc the buffer
            stream->buffer_size += 512;
            stream->buffer = realloc(stream->buffer, stream->buffer_size);
        }
        // we copy the char
        stream->buffer[stream->buffer_pos] = ((char *) buffer)[i];
        // we increment the buffer position
        stream->buffer_pos++;
    }
    // we flush
    fflush(stream);
    // in any case, we return the count
    return count;
}

int fgetc(FILE *stream) {
    puts("fgetc not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int getc(FILE *stream) {
    // we check if the file is null
    if (stream == NULL || stream == stdout) {
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
    // we check if the file is at the end
    if (stream->buffer_pos >= stream->buffer_size) {
        stream->eof = 1;
        return 0;
    }
    // we copy the char
    char c = stream->buffer[stream->buffer_pos];
    // we increment the buffer position
    stream->buffer_pos++;
    return c;
}

char *fgets(char *restrict str, int count, FILE *restrict stream) {
    if (stream == stdin) {
        input(str, count, 0x09);
        puts("\n");
        return str;
    }

    return NULL;
}

int fputc(int ch, FILE *stream) {
    puts("fputc not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int putc(int ch, FILE *stream) {
    puts("putc not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fputs(const char *restrict str, FILE *restrict stream) {
    puts("fputs not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int getchar(void) {
    // Equivalent to getc(stdin). (https://en.cppreference.com/w/c/io/getchar)
    puts("getchar not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *gets_s(char *str, rsize_t n ) {
    puts("gets_s not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

int putchar(int ch) {
    char tmp[2] = {ch, 0};
    puts(tmp);
    return ch;
}

int puts(const char *str) {
    color_print((char *) str);
    return strlen(str);
}

int ungetc(int ch, FILE *stream) {
    puts("ungetc not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int scanf(const char *restrict format, ...) {
    puts("scanf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fscanf(FILE *restrict stream, const char *restrict format, ...) {
    puts("fscanf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int sscanf(const char *restrict buffer, const char *restrict format, ...) {
    puts("sscanf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int scanf_s(const char *restrict format, ...) {
    puts("scanf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fscanf_s(FILE *restrict stream, const char *restrict format, ...) {
    puts("fscanf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int sscanf_s(const char *restrict buffer, const char *restrict format, ...) {
    puts("sscanf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vscanf(const char *restrict format, va_list vlist) {
    puts("vscanf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vfscanf(FILE *restrict stream, const char *restrict format, va_list vlist) {
    puts("vfscanf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vsscanf(const char *restrict buffer, const char *restrict format, va_list vlist) {
    puts("vsscanf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vscanf_s(const char *restrict format, va_list vlist) {
    puts("vscanf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vfscanf_s(FILE *restrict stream, const char *restrict format, va_list vlist) {
    puts("vfscanf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vsscanf_s(const char *restrict buffer, const char *restrict format, va_list vlist) {
    puts("vsscanf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int printf(const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    return 0;
}

int fprintf(FILE *restrict stream, const char *restrict format, ...) {
    c_serial_print(SERIAL_PORT_A, "WARNING: fprintf is not correctly implemented\n");
    // if the stream is stdout or stderr, we use vfsprint
    if (stream == stdout || stream == stderr) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        return strlen(format); // TODO : return the true number of characters written
    }
    // if the stream is read only, we can't write to it
    if (stream == stdin || strcmp(stream->mode, "r") == 0 || strcmp(stream->mode, "r+") == 0) {
        return 0;
    }
    // if the stream is a file, we show an error, it's not implemented yet
    puts("fprintf not implemented for files yet, WHY DO YOU USE IT ?\n");
    return 0; // TODO : return the true number of characters written
}

int sprintf(char *restrict buffer, const char *restrict format, ...) {
    va_list args;
    // we copy format to a buffer because we need to modify it
    va_start(args, format);
    dopr(buffer, -1, format, args);
    va_end(args);
    return 0;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    int r;
    va_list args;
    va_start(args, format);
    r = dopr(str, size, format, args);
    va_end(args);
    return r;
}

int printf_s(const char *restrict format, ...) {
    puts("printf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fprintf_s(FILE *restrict stream, const char *restrict format, ...) {
    puts("fprintf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int sprintf_s(char *restrict buffer, rsize_t bufsz, const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    dopr(buffer, bufsz, format, args);
    va_end(args);
    return 0;
}

int snprintf_s(char *restrict buffer, rsize_t bufsz, const char *restrict format, ...) {
    puts("snprintf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vprintf(const char *restrict format, va_list vlist) {
    int count;

    // use dopr to print the string
    char *buffer = malloc(0x4000);
    dopr(buffer, 0x4000, format, vlist);

    // print the string
    count = puts(buffer);

    free(buffer);
    return count;
}

int vfprintf(FILE *restrict stream, const char *restrict format, va_list vlist) {
    if (stream == stdout || stream == stderr) {
        vprintf(format, vlist);
        return strlen(format); // TODO : return the true number of characters written
    }
    puts("vfprintf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vsprintf(char *restrict buffer, const char *restrict format, va_list vlist) {
    return dopr(buffer, -1, format, vlist);
}

int vsnprintf(char *restrict str, size_t count, const char *restrict fmt, va_list args) {
    return dopr(str, count, fmt, args);
}

int vprintf_s(const char *restrict format, va_list vlist) {
    puts("vprintf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vfprintf_s(FILE *restrict stream, const char *restrict format, va_list vlist) {
    puts("vfprintf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vsprintf_s(char *restrict buffer, rsize_t bufsz, const char *restrict format, va_list vlist) {
    puts("vsprintf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int vsnprintf_s(char *restrict buffer, rsize_t bufsz, const char *restrict format, va_list vlist) {
    puts("vsnprintf_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

long ftell(FILE *stream) {
    if (stream == NULL) {
        return 0;
    }
    return stream->buffer_pos;
}

int feof(FILE *stream) {
    // we check if the file is null
    if (stream == NULL) {
        return 0;
    }
    return stream->eof;
}

int ferror(FILE *stream) {
    c_serial_print(SERIAL_PORT_A, "WARNING: ferror not correctly implemented...\n");
    return 0;   // return 0 if no error found
}

void perror(const char *s) {
    puts("perror not implemented yet, WHY DO YOU USE IT ?\n");
}

int remove(const char *fname) {
    puts("remove not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int rename(const char *old_filename, const char *new_filename) {
    puts("rename not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

FILE *tmpfile(void) {
    puts("tmpfile not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

errno_t tmpfile_s(FILE * restrict * restrict streamptr) {
    puts("tmpfile_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *tmpnam(char *filename) {
    puts("tmpnam not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

errno_t tmpnam_s(char *filename_s, rsize_t maxsize) {
    puts("tmpnam_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

/********************************
 *                             *
 *  INTERNAL FUNCTIONS - DOPR  *
 *                             *
********************************/

/* snprintf - compatibility implementation of snprintf, vsnprintf
 *
 * Copyright (c) 2013, NLnet Labs. All rights reserved.
 *
 * This software is open source.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the NLNET LABS nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Very portable snprintf implementation, limited in functionality,
 * esp. for %[capital] %[nonportable] and so on.  Reduced float functionality,
 * mostly in formatting and range (e+-16), for %f and %g.
 *
 * %s, %d, %u, %i, %x, %c, %n and %% are fully supported.
 *   This includes width, precision, flags 0- +, and *(arg for wid,prec).
 * %f, %g, %m, %p have reduced support, support for wid,prec,flags,*, but
 *   less floating point range, no %e formatting for %g.

 * -- profanOS edit note --
 * snprintf renamed to dopr
 * long long changed to long
*/

/** add padding to string */
static void
print_pad(char** at, size_t* left, int* ret, char p, int num)
{
    while(num--) {
        if(*left > 1) {
            *(*at)++ = p;
            (*left)--;
        }
        (*ret)++;
    }
}

/** get negative symbol, 0 if none */
static char
get_negsign(int negative, int plus, int space)
{
    if(negative)
        return '-';
    if(plus)
        return '+';
    if(space)
        return ' ';
    return 0;
}

#define PRINT_DEC_BUFSZ 32 /* 20 is enough for 64 bit decimals */
/** print decimal into buffer, returns length */
static int
print_dec(char* buf, int max, unsigned int value)
{
    int i = 0;
    if(value == 0) {
        if(max > 0) {
            buf[0] = '0';
            i = 1;
        }
    } else while(value && i < max) {
        buf[i++] = '0' + value % 10;
        value /= 10;
    }
    return i;
}

/** print long decimal into buffer, returns length */
static int
print_dec_l(char* buf, int max, unsigned long value)
{
    int i = 0;
    if(value == 0) {
        if(max > 0) {
            buf[0] = '0';
            i = 1;
        }
    } else while(value && i < max) {
        buf[i++] = '0' + value % 10;
        value /= 10;
    }
    return i;
}

/** print long decimal into buffer, returns length */
static int
print_dec_ll(char* buf, int max, unsigned long value)
{
    int i = 0;
    if(value == 0) {
        if(max > 0) {
            buf[0] = '0';
            i = 1;
        }
    } else while(value && i < max) {
        buf[i++] = '0' + value % 10;
        value /= 10;
    }
    return i;
}

/** print hex into buffer, returns length */
static int
print_hex(char* buf, int max, unsigned int value)
{
    const char* h = "0123456789abcdef";
    int i = 0;
    if(value == 0) {
        if(max > 0) {
            buf[0] = '0';
            i = 1;
        }
    } else while(value && i < max) {
        buf[i++] = h[value & 0x0f];
        value >>= 4;
    }
    return i;
}

/** print long hex into buffer, returns length */
static int
print_hex_l(char* buf, int max, unsigned long value)
{
    const char* h = "0123456789abcdef";
    int i = 0;
    if(value == 0) {
        if(max > 0) {
            buf[0] = '0';
            i = 1;
        }
    } else while(value && i < max) {
        buf[i++] = h[value & 0x0f];
        value >>= 4;
    }
    return i;
}

/** print long hex into buffer, returns length */
static int
print_hex_ll(char* buf, int max, unsigned long value)
{
    const char* h = "0123456789abcdef";
    int i = 0;
    if(value == 0) {
        if(max > 0) {
            buf[0] = '0';
            i = 1;
        }
    } else while(value && i < max) {
        buf[i++] = h[value & 0x0f];
        value >>= 4;
    }
    return i;
}

/** copy string into result, reversed */
static void
spool_str_rev(char** at, size_t* left, int* ret, const char* buf, int len)
{
    int i = len;
    while(i) {
        if(*left > 1) {
            *(*at)++ = buf[--i];
            (*left)--;
        } else --i;
        (*ret)++;
    }
}

/** copy string into result */
static void
spool_str(char** at, size_t* left, int* ret, const char* buf, int len)
{
    int i;
    for(i=0; i<len; i++) {
        if(*left > 1) {
            *(*at)++ = buf[i];
            (*left)--;
        }
        (*ret)++;
    }
}

/** print number formatted */
static void
print_num(char** at, size_t* left, int* ret, int minw, int precision,
    int prgiven, int zeropad, int minus, int plus, int space,
    int zero, int negative, char* buf, int len)
{
    int w = len; /* excludes minus sign */
    char s = get_negsign(negative, plus, space);
    if(minus) {
        /* left adjust the number into the field, space padding */
        /* calc numw = [sign][zeroes][number] */
        int numw = w;
        if(precision == 0 && zero) numw = 0;
        if(numw < precision) numw = precision;
        if(s) numw++;

        /* sign */
        if(s) print_pad(at, left, ret, s, 1);

        /* number */
        if(precision == 0 && zero) {
            /* "" for the number */
        } else {
            if(w < precision)
                print_pad(at, left, ret, '0', precision - w);
            spool_str_rev(at, left, ret, buf, len);
        }
        /* spaces */
        if(numw < minw)
            print_pad(at, left, ret, ' ', minw - numw);
    } else {
        /* pad on the left of the number */
        /* calculate numw has width of [sign][zeroes][number] */
        int numw = w;
        if(precision == 0 && zero) numw = 0;
        if(numw < precision) numw = precision;
        if(!prgiven && zeropad && numw < minw) numw = minw;
        else if(s) numw++;

        /* pad with spaces */
        if(numw < minw)
            print_pad(at, left, ret, ' ', minw - numw);
        /* print sign (and one less zeropad if so) */
        if(s) {
            print_pad(at, left, ret, s, 1);
            numw--;
        }
        /* pad with zeroes */
        if(w < numw)
            print_pad(at, left, ret, '0', numw - w);
        if(precision == 0 && zero)
            return;
        /* print the characters for the value */
        spool_str_rev(at, left, ret, buf, len);
    }
}

/** print %d and %i */
static void
print_num_d(char** at, size_t* left, int* ret, int value,
    int minw, int precision, int prgiven, int zeropad, int minus,
    int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = (value < 0);
    int zero = (value == 0);
    int len = print_dec(buf, (int)sizeof(buf),
        (unsigned int)(negative?-value:value));
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
        plus, space, zero, negative, buf, len);
}

/** print %ld and %li */
static void
print_num_ld(char** at, size_t* left, int* ret, long value,
    int minw, int precision, int prgiven, int zeropad, int minus,
    int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = (value < 0);
    int zero = (value == 0);
    int len = print_dec_l(buf, (int)sizeof(buf),
        (unsigned long)(negative?-value:value));
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
        plus, space, zero, negative, buf, len);
}

/** print %lld and %lli */
static void
print_num_lld(char** at, size_t* left, int* ret, long value,
    int minw, int precision, int prgiven, int zeropad, int minus,
    int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = (value < 0);
    int zero = (value == 0);
    int len = print_dec_ll(buf, (int)sizeof(buf),
        (unsigned long)(negative?-value:value));
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
        plus, space, zero, negative, buf, len);
}

/** print %u */
static void
print_num_u(char** at, size_t* left, int* ret, unsigned int value,
    int minw, int precision, int prgiven, int zeropad, int minus,
    int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
    int len = print_dec(buf, (int)sizeof(buf), value);
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
        plus, space, zero, negative, buf, len);
}

/** print %lu */
static void
print_num_lu(char** at, size_t* left, int* ret, unsigned long value,
    int minw, int precision, int prgiven, int zeropad, int minus,
    int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
    int len = print_dec_l(buf, (int)sizeof(buf), value);
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
        plus, space, zero, negative, buf, len);
}

/** print %llu */
static void
print_num_llu(char** at, size_t* left, int* ret, unsigned long value,
    int minw, int precision, int prgiven, int zeropad, int minus,
    int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
    int len = print_dec_ll(buf, (int)sizeof(buf), value);
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
        plus, space, zero, negative, buf, len);
}

/** print %x */
static void
print_num_x(char** at, size_t* left, int* ret, unsigned int value,
    int minw, int precision, int prgiven, int zeropad, int minus,
    int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
    int len = print_hex(buf, (int)sizeof(buf), value);
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
        plus, space, zero, negative, buf, len);
}

/** print %lx */
static void
print_num_lx(char** at, size_t* left, int* ret, unsigned long value,
    int minw, int precision, int prgiven, int zeropad, int minus,
    int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
    int len = print_hex_l(buf, (int)sizeof(buf), value);
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
        plus, space, zero, negative, buf, len);
}

/** print %llx */
static void
print_num_llx(char** at, size_t* left, int* ret, unsigned long value,
    int minw, int precision, int prgiven, int zeropad, int minus,
    int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
    int len = print_hex_ll(buf, (int)sizeof(buf), value);
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
        plus, space, zero, negative, buf, len);
}

/** print %llp */
static void
print_num_llp(char** at, size_t* left, int* ret, void* value,
    int minw, int precision, int prgiven, int zeropad, int minus,
    int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
#if defined(UINTPTR_MAX) && defined(UINT32_MAX) && (UINTPTR_MAX == UINT32_MAX)
    /* avoid warning about upcast on 32bit systems */
    unsigned long llvalue = (unsigned long)value;
#else
    unsigned long llvalue = (unsigned long)value;
#endif
    int len = print_hex_ll(buf, (int)sizeof(buf), llvalue);
    if(zero) {
        buf[0]=')';
        buf[1]='l';
        buf[2]='i';
        buf[3]='n';
        buf[4]='(';
        len = 5;
    } else {
        /* put '0x' in front of the (reversed) buffer result */
        if(len < PRINT_DEC_BUFSZ)
            buf[len++] = 'x';
        if(len < PRINT_DEC_BUFSZ)
            buf[len++] = '0';
    }
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
        plus, space, zero, negative, buf, len);
}

#define PRINT_FLOAT_BUFSZ 64 /* xx.yy with 20.20 about the max */
/** spool remainder after the decimal point to buffer, in reverse */
static int
print_remainder(char* buf, int max, double r, int prec)
{
    unsigned long cap = 1;
    unsigned long value;
    int len, i;
    if(prec > 19) prec = 19; /* max we can do */
    if(max < prec) return 0;
    for(i=0; i<prec; i++) {
        cap *= 10;
    }
    r *= (double)cap;
    value = (unsigned long)r;
    /* see if we need to round up */
    if(((unsigned long)((r - (double)value)*10.0)) >= 5) {
        value++;
        /* that might carry to numbers before the comma, if so,
         * just ignore that rounding. failure because 64bitprintout */
        if(value >= cap)
            value = cap-1;
    }
    len = print_dec_ll(buf, max, value);
    while(len < prec) { /* pad with zeroes, e.g. if 0.0012 */
        buf[len++] = '0';
    }
    if(len < max)
        buf[len++] = '.';
    return len;
}

/** spool floating point to buffer */
static int
print_float(char* buf, int max, double value, int prec)
{
    /* as xxx.xxx  if prec==0, no '.', with prec decimals after . */
    /* no conversion for NAN and INF, because we do not want to require
       linking with -lm. */
    /* Thus, the conversions use 64bit integers to convert the numbers,
     * which makes 19 digits before and after the decimal point the max */
    unsigned long whole = (unsigned long)value;
    double remain = value - (double)whole;
    int len = 0;
    if(prec != 0)
        len = print_remainder(buf, max, remain, prec);
    len += print_dec_ll(buf+len, max-len, whole);
    return len;
}

/** print %f */
static void
print_num_f(char** at, size_t* left, int* ret, double value,
    int minw, int precision, int prgiven, int zeropad, int minus,
    int plus, int space)
{
    char buf[PRINT_FLOAT_BUFSZ];
    int negative = (value < 0);
    int zero = 0;
    int len;
    if(!prgiven) precision = 6;
    len = print_float(buf, (int)sizeof(buf), negative?-value:value,
        precision);
    print_num(at, left, ret, minw, 1, 0, zeropad, minus,
        plus, space, zero, negative, buf, len);
}

/* rudimentary %g support */
static int
print_float_g(char* buf, int max, double value, int prec)
{
    unsigned long whole = (unsigned long)value;
    double remain = value - (double)whole;
    int before = 0;
    int len = 0;

    /* number of digits before the decimal point */
    while(whole > 0) {
        before++;
        whole /= 10;
    }
    whole = (unsigned long)value;

    if(prec > before && remain != 0.0) {
        /* see if the last decimals are zero, if so, skip them */
        len = print_remainder(buf, max, remain, prec-before);
        while(len > 0 && buf[0]=='0') {
            memmove(buf, buf+1, --len);
        }
    }
    len += print_dec_ll(buf+len, max-len, whole);
    return len;
}


/** print %g */
static void
print_num_g(char** at, size_t* left, int* ret, double value,
    int minw, int precision, int prgiven, int zeropad, int minus,
    int plus, int space)
{
    char buf[PRINT_FLOAT_BUFSZ];
    int negative = (value < 0);
    int zero = 0;
    int len;
    if(!prgiven) precision = 6;
    if(precision == 0) precision = 1;
    len = print_float_g(buf, (int)sizeof(buf), negative?-value:value,
        precision);
    print_num(at, left, ret, minw, 1, 0, zeropad, minus,
        plus, space, zero, negative, buf, len);
}


/** strnlen (compat implementation) */
static int
my_strnlen(const char* s, int max)
{
    int i;
    for(i=0; i<max; i++)
        if(s[i]==0)
            return i;
    return max;
}

/** print %s */
static void
print_str(char** at, size_t* left, int* ret, char* s,
    int minw, int precision, int prgiven, int minus)
{
    if (s == NULL) s = "(null)"; /* unofficial modification */
    int w;
    /* with prec: no more than x characters from this string, stop at 0 */
    if(prgiven)
        w = my_strnlen(s, precision);
    else    w = (int)strlen(s); /* up to the nul */
    if(w < minw && !minus)
        print_pad(at, left, ret, ' ', minw - w);
    spool_str(at, left, ret, s, w);
    if(w < minw && minus)
        print_pad(at, left, ret, ' ', minw - w);
}

/** print %c */
static void
print_char(char** at, size_t* left, int* ret, int c,
    int minw, int minus)
{
    if(1 < minw && !minus)
        print_pad(at, left, ret, ' ', minw - 1);
    print_pad(at, left, ret, c, 1);
    if(1 < minw && minus)
        print_pad(at, left, ret, ' ', minw - 1);
}

/**
 * Print to string.
 * str: string buffer for result. result will be null terminated.
 * size: size of the buffer. null is put inside buffer.
 * format: printf format string.
 * arg: '...' arguments to print.
 * returns number of characters. a null is printed after this.
 * return number of bytes that would have been written
 *       if the buffer had been large enough.
 *
 * supported format specifiers:
 *     %s, %u, %d, %x, %i, %f, %g, %c, %p, %n.
 *     length: l, ll (for d, u, x).
 *     precision: 6.6d (for d, u, x)
 *         %f, %g precisions, 0.3f
 *         %20s, '.*s'
 *     and %%.
 */

int dopr(char* str, size_t size, const char* format, va_list arg) {
    char* at = str;
    size_t left = size;
    int ret = 0;
    const char* fmt = format;
    int conv, minw, precision, prgiven, zeropad, minus, plus, space, length;
    while(*fmt) {
        /* copy string before % */
        while(*fmt && *fmt!='%') {
            if(left > 1) {
                *at++ = *fmt++;
                left--;
            } else fmt++;
            ret++;
        }

        /* see if we are at end */
        if(!*fmt) break;

        /* fetch next argument % designation from format string */
        fmt++; /* skip the '%' */

        /********************************/
        /* get the argument designation */
        /********************************/
        /* we must do this vararg stuff inside this function for
         * portability.  Hence, get_designation, and print_designation
         * are not their own functions. */

        /* printout designation:
         * conversion specifier: x, d, u, s, c, n, m, p
         * flags: # not supported
         *        0 zeropad (on the left)
         *      - left adjust (right by default)
         *      ' ' printspace for positive number (in - position).
         *      + alwayssign
         * fieldwidth: [1-9][0-9]* minimum field width.
         *     if this is * then type int next argument specifies the minwidth.
         *     if this is negative, the - flag is set (with positive width).
         * precision: period[digits]*, %.2x.
         *     if this is * then type int next argument specifies the precision.
         *    just '.' or negative value means precision=0.
         *        this is mindigits to print for d, i, u, x
         *        this is aftercomma digits for f
         *        this is max number significant digits for g
         *        maxnumber characters to be printed for s
         * length: 0-none (int), 1-l (long), 2-ll (long)
         *     notsupported: hh (char), h (short), L (long double), q, j, z, t
         * Does not support %m$ and *m$ argument designation as array indices.
         * Does not support %#x
         *
         */
        minw = 0;
        precision = 1;
        prgiven = 0;
        zeropad = 0;
        minus = 0;
        plus = 0;
        space = 0;
        length = 0;

        /* get flags in any order */
        for(;;) {
            if(*fmt == '0')
                zeropad = 1;
            else if(*fmt == '-')
                minus = 1;
            else if(*fmt == '+')
                plus = 1;
            else if(*fmt == ' ')
                space = 1;
            else break;
            fmt++;
        }

        /* field width */
        if(*fmt == '*') {
            fmt++; /* skip char */
            minw = va_arg(arg, int);
            if(minw < 0) {
                minus = 1;
                minw = -minw;
            }
        } else while(*fmt >= '0' && *fmt <= '9') {
            minw = minw*10 + (*fmt++)-'0';
        }

        /* precision */
        if(*fmt == '.') {
            fmt++; /* skip period */
            prgiven = 1;
            precision = 0;
            if(*fmt == '*') {
                fmt++; /* skip char */
                precision = va_arg(arg, int);
                if(precision < 0)
                    precision = 0;
            } else while(*fmt >= '0' && *fmt <= '9') {
                precision = precision*10 + (*fmt++)-'0';
            }
        }

        /* length */
        if(*fmt == 'l') {
            fmt++; /* skip char */
            length = 1;
            if(*fmt == 'l') {
                fmt++; /* skip char */
                length = 2;
            }
        }

        /* get the conversion */
        if(!*fmt) conv = 0;
        else    conv = *fmt++;

        /***********************************/
        /* print that argument designation */
        /***********************************/
        switch(conv) {
        case 'i':
        case 'd':
            if(length == 0)
                print_num_d(&at, &left, &ret, va_arg(arg, int),
                minw, precision, prgiven, zeropad, minus, plus, space);
            else if(length == 1)
                print_num_ld(&at, &left, &ret, va_arg(arg, long),
                minw, precision, prgiven, zeropad, minus, plus, space);
            else if(length == 2)
                print_num_lld(&at, &left, &ret,
                va_arg(arg, long),
                minw, precision, prgiven, zeropad, minus, plus, space);
            break;
        case 'u':
            if(length == 0)
                print_num_u(&at, &left, &ret,
                va_arg(arg, unsigned int),
                minw, precision, prgiven, zeropad, minus, plus, space);
            else if(length == 1)
                print_num_lu(&at, &left, &ret,
                va_arg(arg, unsigned long),
                minw, precision, prgiven, zeropad, minus, plus, space);
            else if(length == 2)
                print_num_llu(&at, &left, &ret,
                va_arg(arg, unsigned long),
                minw, precision, prgiven, zeropad, minus, plus, space);
            break;
        case 'x':
            if(length == 0)
                print_num_x(&at, &left, &ret,
                va_arg(arg, unsigned int),
                minw, precision, prgiven, zeropad, minus, plus, space);
            else if(length == 1)
                print_num_lx(&at, &left, &ret,
                va_arg(arg, unsigned long),
                minw, precision, prgiven, zeropad, minus, plus, space);
            else if(length == 2)
                print_num_llx(&at, &left, &ret,
                va_arg(arg, unsigned long),
                minw, precision, prgiven, zeropad, minus, plus, space);
            break;
        case 's':
            print_str(&at, &left, &ret, va_arg(arg, char*),
                minw, precision, prgiven, minus);
            break;
        case 'c':
            print_char(&at, &left, &ret, va_arg(arg, int),
                minw, minus);
            break;
        case 'n':
            *va_arg(arg, int*) = ret;
            break;
        case 'm':
            /*
            print_str(&at, &left, &ret, strerror(errno),
                minw, precision, prgiven, minus);
            */
            print_str(&at, &left, &ret, "strerror(errno)",
                minw, precision, prgiven, minus);
            break;
        case 'p':
            print_num_llp(&at, &left, &ret, va_arg(arg, void*),
                minw, precision, prgiven, zeropad, minus, plus, space);
            break;
        case '%':
            print_pad(&at, &left, &ret, '%', 1);
            break;
        case 'f':
            print_num_f(&at, &left, &ret, va_arg(arg, double),
                minw, precision, prgiven, zeropad, minus, plus, space);
            break;
        case 'g':
            print_num_g(&at, &left, &ret, va_arg(arg, double),
                minw, precision, prgiven, zeropad, minus, plus, space);
            break;
        /* unknown */
        default:
        case 0: break;
        }
    }

    /* zero terminate */
    if(left > 0)
        *at = 0;
    return ret;
}
