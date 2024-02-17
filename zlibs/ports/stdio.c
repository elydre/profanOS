#include <filesys.h>
#include <syscall.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <type.h>

#include <stdarg.h>

#define STDIO_C
#include <stdio.h>

#define FILE_BUFFER_SIZE 0x1000
#define FILE_BUFFER_READ 100

#define MODE_READ   1 << 0
#define MODE_WRITE  1 << 1
#define MODE_APPEND 1 << 2

void init_func();

int puts(const char *str);
int fflush(FILE *stream);
int fclose(FILE *stream);

int dopr(char* str, size_t size, const char* format, va_list arg);
int vfprintf(FILE *stream, const char *format, va_list vlist);
int vprintf(const char *format, va_list vlist);
int printf(const char *format, ...);

FILE *STD_STREAM = NULL;

int main(void) {
    init_func();
    return 0;
}

void init_func(void) {
    FILE *dup = calloc(3, sizeof(FILE));

    // init stdin
    dup[0].filename = "/dev/stdin";
    dup[0].mode = MODE_READ;
    dup[0].buffer = NULL;
    dup[0].fd = 0;

    // init stdout
    dup[1].filename = "/dev/stdout";
    dup[1].mode = MODE_WRITE;
    dup[1].buffer = malloc(FILE_BUFFER_SIZE);
    dup[1].fd = 1;

    // init stderr
    dup[2].filename = "/dev/stderr";
    dup[2].mode = MODE_WRITE;
    dup[2].buffer = malloc(FILE_BUFFER_SIZE);
    dup[2].fd = 2;

    STD_STREAM = dup;
}

void clearerr(FILE *stream) {
    puts("clearerr not implemented yet, WHY DO YOU USE IT ?");
}

FILE *fopen(const char *filename, const char *mode) {
    // check for null pointers
    if (filename == NULL || mode == NULL) {
        return NULL;
    }

    // if path is relative, add the current directory
    char *path;
    if (filename[0] != '/') {
        char *pwd = getenv("PWD");
        if (pwd == NULL) pwd = "/";

        path = assemble_path(pwd, (char *) filename);
        filename = path;
    } else {
        path = strdup(filename);
    }

    // compute the mode
    uint32_t interpeted_mode = 0;
    for (uint32_t i = 0; i < strlen(mode); i++) {
        switch (mode[i]) {
            case 'r':
                interpeted_mode |= MODE_READ;
                break;
            case 'w':
                interpeted_mode |= MODE_WRITE;
                break;
            case 'a':
                interpeted_mode |= MODE_APPEND | MODE_WRITE;
                break;
            case '+':
                interpeted_mode |= MODE_READ | MODE_WRITE;
                break;
        }
    }

    // first check if the file exists
    sid_t file_id = fu_path_to_sid(ROOT_SID, (char *) path);
    int exists = !IS_NULL_SID(file_id);

    // the file doesn't exist but it should
    if (!exists && !(interpeted_mode & MODE_WRITE)) {
        free(path);
        return NULL;
    }

    // the path is a directory
    if (exists && fu_is_dir(file_id)) {
        free(path);
        return NULL;
    }

    // create the file if it doesnt exist
    if (!exists) {
        file_id = fu_file_create(0, (char *) path);
    }

    // check for failure
    if (IS_NULL_SID(file_id)) {
        free(path);
        return NULL;
    }

    // now create the file structure
    FILE *file = malloc(sizeof(FILE));

    // copy the filename
    file->filename = path;
    file->fd = fm_open((char *) path);
    if (file->fd < 0) {
        free(file);
        return NULL;
    }

    // copy the mode
    file->mode = interpeted_mode;

    // set the buffer
    file->buffer = malloc(FILE_BUFFER_SIZE);
    file->buffer_size = 0;
    file->buffer_pid = -1;
    file->old_offset = 0;

    // if the file is open for appending, set the file pos to the end of the file
    if (interpeted_mode & MODE_APPEND)
        fm_lseek(file->fd, 0, SEEK_END);

    return file;
}

errno_t fopen_s(FILE **streamptr, const char *filename, const char *mode) {
    puts("fopen_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

FILE *freopen(const char *filename, const char *mode, FILE *stream) {
    if (stream == stdin || stream == stdout || stream == stderr) {
        return NULL;
    }

    // close the file
    fclose(stream);
    // open the file
    return fopen(filename, mode);
}

errno_t freopen_s(FILE **newstreamptr, const char *filename, const char *mode, FILE *stream) {
    if (stream == stdin || stream == stdout || stream == stderr) {
        return 0;
    }

    // close the file
    fclose(stream);
    // open the file
    *newstreamptr = fopen(filename, mode);
    return 0;
}

int fclose(FILE *stream) {
    // check for stdin, stdout and stderr
    if (stream == stdin || stream == stdout || stream == stderr) {
        return 0;
    }

    // fflush the stream
    fflush(stream);

    // close the file
    fm_close(stream->fd);

    // free the stream
    free(stream->filename);
    free(stream->buffer);
    free(stream);

    return 0;
}

int fflush(FILE *stream) {
    // check if the file is stdin
    if (stream == stdin) {
        return 0;
    }

    if (stream == stdout || stream == stderr) {
        stream = STD_STREAM + (stream == stdout ? 1 : 2);
    }

    // check if the file is open for writing, else return 0
    if (!(stream->mode & MODE_WRITE)) return 0;
    if (stream->buffer_size <= 0) return 0;

    uint32_t buffer_size = stream->buffer_size;
    stream->buffer_size = 0;

    stream->buffer[buffer_size] = 0;

    int fd = stream->fd;
    if (fd < 3)
        fd = fm_resol012(fd, stream->buffer_pid);

    // write the file
    int written = fm_write(fd, stream->buffer, buffer_size);
    if (written < 0) written = 0;

    // return the number of elements written
    return written ? 0 : EOF;
}

void setbuf(FILE *stream, char *buffer) {
    puts("setbuf not implemented yet, WHY DO YOU USE IT ?");
}

int setvbuf(FILE *stream, char *buffer, int mode, size_t size) {
    puts("setvbuf not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int fwide(FILE *stream, int mode) {
    puts("fwide not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

size_t fread(void *buffer, size_t size, size_t count, FILE *stream) {
    // return if total size is 0
    count *= size;
    if (count == 0) return 0;

    // check if the file is stdout or stderr
    if (stream == stdout || stream == stderr) {
        return 0;
    }

    if (stream == stdin) {
        stream = STD_STREAM + 0;
    }

    // check if the file is open for reading, else return 0
    if (!(stream->mode & MODE_READ)) return 0;

    fflush(stream);

    int read, rfrom_buffer = 0;

    if (fm_isfctf(stream->fd)) {
        // read the file
        read = fm_read(stream->fd, buffer, count);
        if (read < 0) return 0;

        // return the number of elements read
        return read / size;
    }

    // read the file from the buffer if possible
    if (stream->buffer_size < 0 && stream->old_offset == fm_tell(stream->fd)) {
        if ((uint32_t) -stream->buffer_size > count) {
            memcpy(buffer, stream->buffer, count);
            stream->buffer_size += count;

            // move the buffer
            memmove(stream->buffer, stream->buffer + count, -stream->buffer_size);
            return count / size;
        }

        memcpy(buffer, stream->buffer, -stream->buffer_size);
        rfrom_buffer = -stream->buffer_size;
        count += stream->buffer_size;

        stream->buffer_size = 0;
    }

    // read the file
    if (count > FILE_BUFFER_READ) {
        read = fm_read(stream->fd, buffer + rfrom_buffer, count);
        if (read < 0) return 0;
        return (read + rfrom_buffer) / size;
    }

    read = fm_read(stream->fd, stream->buffer, FILE_BUFFER_READ);
    if (read < 0) return 0;
    if ((uint32_t) read < count)
        count = read;

    memcpy(buffer + rfrom_buffer, stream->buffer, count);
    memmove(stream->buffer, stream->buffer + count, read - count);
    stream->buffer_size = -(read - count);

    stream->old_offset = fm_tell(stream->fd);

    // return the number of elements read
    return (count + rfrom_buffer) / size;
}

size_t fwrite(const void *buffer, size_t size, size_t count, FILE *stream) {
    int pid;

    // return if total size is 0
    count *= size;
    if (count == 0) return 0;

    // check if the file is stdin
    if (stream == stdin) {
        return 0;
    }

    if (stream == stdout || stream == stderr) {
        stream = STD_STREAM + (stream == stdout ? 1 : 2);
    }

    // check if the file is open for writing, else return 0
    if (!(stream->mode & MODE_WRITE)) return 0;

    // if buffer is used for reading
    if (stream->buffer_size < 0) {
        stream->buffer_size = 0;
    }

    if (stream->buffer_pid != -1 && stream->buffer_pid != (pid = c_process_get_pid())) {
        if (stream->buffer_size > 0) {
            fflush(stream);
        }
        stream->buffer_pid = pid;
    }

    // write in the buffer
    uint32_t ret = count;
    int need_flush = 0;

    for (uint32_t i = 0; i < count; i++) {
        // check if the buffer is full
        if (stream->buffer_size >= (FILE_BUFFER_SIZE - 1)) {
            need_flush = 0;
            if (fflush(stream) == EOF) {
                ret = 0;
                break;
            }
        }

        if (((char *) buffer)[i] == '\n') {
            need_flush = 1;
        }

        // write the character
        stream->buffer[stream->buffer_size++] = ((char *) buffer)[i];
    }

    // flush the buffer if needed
    if (need_flush && fflush(stream) == EOF) {
        ret = 0;
    }

    // return the number of elements written
    return ret / size;
}

int fseek(FILE *stream, long offset, int whence) {
    // check if the file is stdin
    if (stream == stdin)
        stream = STD_STREAM + 0;
    else if (stream == stdout)
        stream = STD_STREAM + 1;
    else if (stream == stderr)
        stream = STD_STREAM + 2;

    // flush the buffer
    fflush(stream);

    // set the file position
    return fm_lseek(stream->fd, offset, whence) < 0 ? -1 : 0;
}

int fgetc(FILE *stream) {
    int c;
    return fread(&c, 1, 1, stream) == 1 ? c : EOF;
}

int getc(FILE *stream) {
    int c;
    return fread(&c, 1, 1, stream) == 1 ? c : EOF;
}

char *fgets(char *str, int count, FILE *stream) {
    if (count <= 0 || stream == stdout || stream == stderr) {
        return NULL;
    }

    if (stream == stdin) {
        stream = STD_STREAM + 0;
    }

    size_t rcount = fread(str, 1, count - 1, stream);
    if (rcount == 0) {
        return NULL;
    }

    for (size_t i = 0; i < rcount; i++) {
        if (str[i] == '\n') {
            str[i + 1] = 0;
            // stream->file_pos -= rcount - i - 1;
            fm_lseek(stream->fd, -rcount + i + 1, SEEK_CUR);
            return str;
        }
    }

    str[rcount] = 0;
    return str;
}

int fputc(int ch, FILE *stream) {
    return fwrite(&ch, 1, 1, stream) == 1 ? ch : EOF;
}

int putc(int ch, FILE *stream) {
    return fwrite(&ch, 1, 1, stream) == 1 ? ch : EOF;
}

int fputs(const char *str, FILE *stream) {
    uint32_t len = strlen(str);
    return fwrite(str, 1, len, stream) == len ? 0 : EOF;
}

int getchar(void) {
    return getc(stdin);
}

char *gets_s(char *str, rsize_t n) {
    if (n == 0) {
        return NULL;
    }

    char *ptr = str;
    int c;
    while ((c = getc(stdin)) != EOF && c != '\n') {
        if (n > 1) {
            *ptr++ = c;
            n--;
        }
    }

    if (c == EOF && ptr == str) {
        return NULL;
    }

    *ptr = 0;
    return str;
}

int putchar(int ch) {
    fwrite(&ch, 1, 1, stdout);
    return ch;
}

int puts(const char *str) {
    uint32_t len = strlen(str);
    return (fwrite(str, 1, len, stdout) == len &&
            fwrite("\n", 1, 1, stdout) == 1
        ) ? 0 : EOF;
}

int ungetc(int ch, FILE *stream) {
    puts("ungetc not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int scanf(const char *format, ...) {
    puts("scanf not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int fscanf(FILE *stream, const char *format, ...) {
    puts("fscanf not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int sscanf(const char *buffer, const char *format, ...) {
    puts("sscanf not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int scanf_s(const char *format, ...) {
    puts("scanf_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int fscanf_s(FILE *stream, const char *format, ...) {
    puts("fscanf_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int sscanf_s(const char *buffer, const char *format, ...) {
    puts("sscanf_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int vscanf(const char *format, va_list vlist) {
    puts("vscanf not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int vfscanf(FILE *stream, const char *format, va_list vlist) {
    puts("vfscanf not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int vsscanf(const char *buffer, const char *format, va_list vlist) {
    puts("vsscanf not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int vscanf_s(const char *format, va_list vlist) {
    puts("vscanf_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int vfscanf_s(FILE *stream, const char *format, va_list vlist) {
    puts("vfscanf_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int vsscanf_s(const char *buffer, const char *format, va_list vlist) {
    puts("vsscanf_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int printf(const char *format, ...) {
    int count;
    va_list args;

    va_start(args, format);
    count = vprintf(format, args);
    va_end(args);

    return count;
}

int fprintf(FILE *stream, const char *format, ...) {
    int count;
    va_list args;

    va_start(args, format);
    count = vfprintf(stream, format, args);
    va_end(args);

    return count;
}

int sprintf(char *buffer, const char *format, ...) {
    int count;
    va_list args;

    va_start(args, format);
    count = dopr(buffer, -1, format, args);
    va_end(args);

    return count;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    int count;
    va_list args;

    va_start(args, format);
    count = dopr(str, size, format, args);
    va_end(args);

    return count;
}

int printf_s(const char *format, ...) {
    puts("printf_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int fprintf_s(FILE *stream, const char *format, ...) {
    puts("fprintf_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int sprintf_s(char *buffer, rsize_t bufsz, const char *format, ...) {
    int count;
    va_list args;

    va_start(args, format);
    count = dopr(buffer, bufsz, format, args);
    va_end(args);

    return count;
}

int snprintf_s(char *buffer, rsize_t bufsz, const char *format, ...) {
    puts("snprintf_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int vprintf(const char *format, va_list vlist) {
    int count;

    char *buffer = malloc(0x4000);
    dopr(buffer, 0x4000, format, vlist);
    count = fputs(buffer, stdout);

    free(buffer);
    return count;
}

int vfprintf(FILE *stream, const char *format, va_list vlist) {
    // check if the file is stdin
    if (stream == stdin) {
        return 0;
    }

    // check if the file is stdout or stderr
    if (stream == stdout || stream == stderr) {
        stream = STD_STREAM + (stream == stdout ? 1 : 2);
    }

    // if the stream is read only, can't write to it
    if (!(stream->mode & MODE_WRITE)) {
        return 0;
    }

    // allocate a buffer to store the formatted string
    char *buffer = malloc(0x4000);

    // copy format to a buffer because need to modify it
    int count = dopr(buffer, 0x4000, format, vlist);

    // write the string
    fwrite(buffer, 1, strlen(buffer), stream);

    free(buffer);
    return 0;
}

int vsprintf(char *buffer, const char *format, va_list vlist) {
    return dopr(buffer, -1, format, vlist);
}

int vsnprintf(char *str, size_t count, const char *fmt, va_list args) {
    return dopr(str, count, fmt, args);
}

int vprintf_s(const char *format, va_list vlist) {
    puts("vprintf_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int vfprintf_s(FILE *stream, const char *format, va_list vlist) {
    puts("vfprintf_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int vsprintf_s(char *buffer, rsize_t bufsz, const char *format, va_list vlist) {
    puts("vsprintf_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int vsnprintf_s(char *buffer, rsize_t bufsz, const char *format, va_list vlist) {
    puts("vsnprintf_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long ftell(FILE *stream) {
    // check if the file is stdout or stderr
    if (stream == stdin)
        stream = STD_STREAM;
    else if (stream == stdout)
        stream = STD_STREAM + 1;
    else if (stream == stderr)
        stream = STD_STREAM + 2;

    return fm_tell(stream->fd);
}

int feof(FILE *stream) {
    // check if the file is null
    if (stream == NULL) {
        return 0;
    }

    // check if the file is stdout or stderr
    if (stream == stdout || stream == stderr || stream == stdin) {
        return 0;
    }

    // check if the file is at the end
    uint32_t file_pos = fm_lseek(stream->fd, 0, SEEK_CUR);
    uint32_t file_size = fm_lseek(stream->fd, 0, SEEK_END);
    fm_lseek(stream->fd, file_pos, SEEK_SET);

    return file_pos >= file_size;
}

int ferror(FILE *stream) {
    c_serial_print(SERIAL_PORT_A, "WARNING: ferror not correctly implemented...\n");
    return 0;   // return 0 if no error found
}

void perror(const char *s) {
    puts("perror not implemented yet, WHY DO YOU USE IT ?");
}

int remove(const char *fname) {
    char *pwd = getenv("PWD");
    char *full_path = assemble_path(pwd, (char *) fname);

    sid_t elem = fu_path_to_sid(ROOT_SID, full_path);
    if (IS_NULL_SID(elem) || !fu_is_file(elem)) {
        printf("remove: cannot remove '%s': No such file\n", fname);
        free(full_path);
        return 1;
    }

    char *parent;

    fu_sep_path(full_path, &parent, NULL);

    sid_t parent_sid = fu_path_to_sid(ROOT_SID, parent);
    free(parent);

    if (IS_NULL_SID(parent_sid)) {
        printf("remove: cannot remove '%s': Unreachable path\n", fname);
        free(full_path);
        return 1;
    }

    // remove element from directory
    if (fu_remove_element_from_dir(parent_sid, elem)) {
        printf("remove: cannot remove '%s': Failed to remove element from directory\n", fname);
        return 1;
    }

    // delete container
    if (c_fs_cnt_delete(c_fs_get_main(), elem)) {
        printf("rm: cannot remove '%s': Failed to delete container\n", fname);
        return 1;
    }

    free(full_path);
    return 0;
}

int rename(const char *old_filename, const char *new_filename) {
    puts("rename not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

FILE *tmpfile(void) {
    puts("tmpfile not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

errno_t tmpfile_s(FILE * * streamptr) {
    puts("tmpfile_s not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *tmpnam(char *filename) {
    puts("tmpnam not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

errno_t tmpnam_s(char *filename_s, rsize_t maxsize) {
    if (maxsize < 12) {
        return 1;
    }

    strcpy(filename_s, "/tmp/");
    filename_s[11] = 0;
    do {
        for (int i = 5; i < 11; i++) {
            filename_s[i] = 'a' + rand() % 26;
        }
    } while (!IS_NULL_SID(fu_path_to_sid(ROOT_SID, filename_s)));

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
    if(prec > 19) prec = 19; /* max can do */
    if(max < prec) return 0;
    for(i=0; i<prec; i++) {
        cap *= 10;
    }
    r *= (double)cap;
    value = (unsigned long)r;
    /* see if need to round up */
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
    /* no conversion for NAN and INF, because do not want to require
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

        /* see if are at end */
        if(!*fmt) break;

        /* fetch next argument % designation from format string */
        fmt++; /* skip the '%' */

        /********************************/
        /* get the argument designation */
        /********************************/
        /* must do this vararg stuff inside this function for
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
