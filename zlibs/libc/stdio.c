/*****************************************************************************\
|   === stdio.c : 2024 ===                                                    |
|                                                                             |
|    Implementation of stdio functions from libC                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <modules/filesys.h>
#include <profan/syscall.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <wchar.h>

#include "config_libc.h"

#if STDIO_BUFFER_SIZE < STDIO_BUFFER_READ
  #error "stdio buffer size must be changed"
#endif

typedef struct _IO_FILE {
    int     fd;
    int     mode;

    uint8_t error;
    uint8_t eof;
    uint8_t unbuffered;

    int     ungetchar;
    int     pos;

    char   *buffer;
    int     buffer_size;
    int     buffer_needle;
} FILE;

FILE *stdin = NULL;
FILE *stdout = NULL;
FILE *stderr = NULL;

static char *g_printf_buffer = NULL;

static FILE *fdopen_mode(int fd, int mode) {
    FILE *file = calloc(sizeof(FILE) + STDIO_BUFFER_SIZE, 1);

    file->buffer = ((char *) file) + sizeof(FILE);
    file->ungetchar = -1;
    file->mode = mode;
    file->fd = fd;
    file->unbuffered = !fm_isfile(fd);
    file->pos = fm_lseek(fd, 0, SEEK_CUR);

    return file;
}

static int interpet_mode(const char *mode) {
    // compute the mode
    int fdmode = 0;

    for (int i = 0; mode[i]; i++) {
        switch (mode[i]) {
            case 'r':
                fdmode |= O_RDONLY;
                break;
            case 'w':
                fdmode |= O_WRONLY | O_CREAT | O_TRUNC;
                break;
            case 'a':
                fdmode |= O_WRONLY | O_CREAT | O_APPEND;
                break;
            case '+':
                fdmode |= O_RDWR | O_CREAT;
                break;
        }
    }

    if (fdmode & O_RDWR)
        fdmode &= ~(O_RDONLY | O_WRONLY);

    return fdmode | O_NODIR;
}

void __stdio_init(void) {
    // init stdin, stdout and stderr
    stdin  = fdopen_mode(0, O_RDONLY);
    stdout = fdopen_mode(1, O_WRONLY);
    stderr = fdopen_mode(2, O_WRONLY);

    // init printf buffer
    g_printf_buffer = malloc(0x1000);
}

void __stdio_fini(void) {
    fflush(stdin);
    free(stdin);

    fflush(stdout);
    free(stdout);

    fflush(stderr);
    free(stderr);

    free(g_printf_buffer);
}

void clearerr(FILE *stream) {
    if (stream == NULL)
        return;
    stream->error = 0;
    stream->eof = 0;
}

int fileno(FILE *stream) {
    if (stream == NULL)
        return -1;

    return stream->fd;
}

FILE *fopen(const char *filename, const char *mode) {
    if (filename == NULL || mode == NULL)
        return NULL;

    int fdmode = interpet_mode(mode);

    // open the file
    int fd = open(filename, fdmode, 0666);

    if (fd < 0)
        return NULL;

    return fdopen_mode(fd, fdmode);
}

FILE *fdopen(int fd, const char *mode) {
    if (fd < 0 || mode == NULL)
        return NULL;

    return fdopen_mode(fd, interpet_mode(mode));
}

FILE *freopen(const char *filename, const char *mode, FILE *stream) {
    // close the file
    fclose(stream);

    // open the file
    return fopen(filename, mode);
}

int fclose(FILE *stream) {
    if (stream == NULL)
        return EOF;

    if (stream == stdin)
        stdin = NULL;
    if (stream == stdout)
        stdout = NULL;
    if (stream == stderr)
        stderr = NULL;

    // fflush the stream
    fflush(stream);

    // close the file
    fm_close(stream->fd);

    // free the stream
    free(stream);

    return 0;
}

int fflush(FILE *stream) {
    if (stream == NULL || (stream->mode & 0b11) == O_RDONLY)
        return 0;

    // fflush also sync the file position
    if (stream->buffer_size <= 0) {
        fm_lseek(stream->fd, stream->pos, SEEK_SET);
        return 0;
    }

    uint32_t buffer_size = stream->buffer_size;
    stream->buffer_size = 0;

    stream->buffer[buffer_size] = 0;

    // write the file
    int written = fm_pwrite(stream->fd, stream->buffer, buffer_size, stream->pos);
    stream->pos += written;

    if (written < 0) {
        stream->error = 1;
        written = 0;
    }

    fm_lseek(stream->fd, stream->pos, SEEK_SET);

    // return the number of elements written
    return written ? 0 : EOF;
}

void setbuf(FILE *stream, char *buffer) {
    PROFAN_FNI;
}

int setvbuf(FILE *stream, char *buffer, int mode, size_t size) {
    return (PROFAN_FNI, 0);
}

int fwide(FILE *stream, int mode) {
    return (PROFAN_FNI, 0);
}

size_t fread(void *buffer, size_t size, size_t count, FILE *stream) {
    count *= size;

    // check if the file is open for reading
    if (count == 0 || stream == NULL || (stream->mode & 0b11) == O_WRONLY)
        return 0;

    // if buffer is used for writing
    if (stream->buffer_size > 0)
        fflush(stream);

    int read, rfrom_buffer = 0;

    // check if the file is a function call
    if (stream->unbuffered) {
        read = fm_pread(stream->fd, buffer, count, stream->pos);
        stream->pos += read;

        if (read < 0) {
            stream->error = 1;
            return 0;
        }

        if ((uint32_t) read < count)
            stream->eof = 1;

        return read / size;
    }

    // read the file from the buffer if possible
    if (stream->buffer_size < 0) {
        if ((int) count + stream->buffer_needle <= -stream->buffer_size) {
            memcpy(buffer, stream->buffer + stream->buffer_needle, count);
            stream->buffer_needle += count;
            stream->pos += count;

            return count / size;
        }

        rfrom_buffer = -(stream->buffer_size + stream->buffer_needle);

        if (rfrom_buffer)
            memcpy(buffer, stream->buffer + stream->buffer_needle, rfrom_buffer);

        stream->buffer_size = 0;
        stream->pos += rfrom_buffer;
        count -= rfrom_buffer;
    }

    // read the file
    if (count > STDIO_BUFFER_READ) {
        read = fm_pread(stream->fd, buffer + rfrom_buffer, count, stream->pos);
        stream->pos += read;

        if (read < 0)
            return (stream->error = 1, 0);

        if ((uint32_t) read < count)
            stream->eof = 1;

        return (read + rfrom_buffer) / size;
    }

    read = fm_pread(stream->fd, stream->buffer, STDIO_BUFFER_READ, stream->pos);

    if (read < 0)
        return (stream->error = 1, 0);

    if ((uint32_t) read < count) {
        stream->eof = 1;
        count = read;
    }

    memcpy(buffer + rfrom_buffer, stream->buffer, count);
    stream->buffer_needle = count;
    stream->buffer_size = -read;

    stream->pos += count;

    // return the number of elements read
    return (count + rfrom_buffer) / size;
}

size_t fwrite(const void *buffer, size_t size, size_t count, FILE *stream) {
    count *= size;

    // check if the file is open for writing
    if (count == 0 || stream == NULL || stream->mode & O_RDONLY)
        return 0;

    // if buffer is used for reading
    if (stream->buffer_size < 0)
        stream->buffer_size = 0;

    // write in the buffer
    int need_flush = 0;

    for (uint32_t i = 0; i < count; i++) {
        // check if the buffer is full
        if (stream->buffer_size >= (STDIO_BUFFER_SIZE - 1)) {
            if (fflush(stream) == EOF)
                return 0;
            need_flush = 0;
        }

        if (((char *) buffer)[i] == '\n') {
            need_flush = 1;
        }

        // write the character
        stream->buffer[stream->buffer_size++] = ((char *) buffer)[i];
    }

    // flush the buffer if needed
    if (need_flush && fflush(stream) == EOF)
        return 0;

    return count / size;
}

int fseek(FILE *stream, long offset, int whence) {
    if (stream == NULL)
        return -1;

    // flush the buffer
    fflush(stream);

    // reset the buffer
    stream->buffer_size = 0;
    stream->ungetchar = -1;
    stream->error = 0;
    stream->eof = 0;

    // set the file position
    int newpos = fm_lseek(stream->fd, offset, whence);
    if (newpos < 0)
        return -1;

    stream->pos = newpos;

    return 0;
}

long ftell(FILE *stream) {
    if (stream == NULL)
        return -1;

    // flush the buffer and sync position
    fflush(stream);

    return stream->pos;
}

int fgetpos(FILE *stream, fpos_t *pos) {
    if (pos == NULL)
        return -1;
    return (*pos = ftell(stream)) < 0 ? -1 : 0;
}

int fsetpos(FILE *stream, const fpos_t *pos) {
    if (pos == NULL)
        return -1;
    return fseek(stream, *pos, SEEK_SET);
}

int fgetc(FILE *stream) {
    uint8_t c;
    int val;

    if (stream->ungetchar >= 0) {
        val = stream->ungetchar;
        stream->ungetchar = -1;
        return val;
    }

    return fread(&c, 1, 1, stream) == 1 ? c : EOF;
}

int getc(FILE *stream) {
    return fgetc(stream);
}

char *fgets(char *str, int count, FILE *stream) {
    uint8_t c;

    if (count <= 0)
        return NULL;

    for (int i = 0; i < count - 1; i++) {
        if (fread(&c, 1, 1, stream) != 1) {
            if (i == 0)
                return NULL;
            str[i] = 0;
            return str;
        }

        str[i] = c;
        if (c == '\n') {
            str[i + 1] = 0;
            return str;
        }
    }

    str[count - 1] = 0;
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
    return fwrite(str, 1, len, stream) == len ? (int) len : EOF;
}

int getchar(void) {
    // equivalent to fgetc(stdin)
    char c;
    return fread(&c, 1, 1, stdin) == 1 ? c : EOF;
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

ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream) {
    if (lineptr == NULL || n == NULL || stream == NULL ||
            (stream->mode & 0b11) == O_WRONLY
    ) return -1;

    size_t i = 0;
    uint8_t c;

    while (fread(&c, 1, 1, stream)) {
        if (*lineptr == NULL) {
            *lineptr = malloc(100);
            *n = 100;
        }

        if (i >= *n) {
            *n *= 2;
            *lineptr = realloc(*lineptr, *n);
        }

        (*lineptr)[i++] = c;
        if (c == delim) {
            break;
        }
    }

    if (i == 0) {
        return -1;
    }

    (*lineptr)[i] = 0;
    return i;
}

ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    return getdelim(lineptr, n, '\n', stream);
}

int ungetc(int ch, FILE *stream) {
    if (stream == NULL || stream->ungetchar >= 0 || ch == EOF)
        return EOF;
    stream->ungetchar = ch;
    stream->eof = 0;
    return ch;
}

int scanf(const char *format, ...) {
    return (PROFAN_FNI, 0);
}

int fscanf(FILE *stream, const char *format, ...) {
    return (PROFAN_FNI, 0);
}

int sscanf(const char *buffer, const char *format, ...) {
    va_list args;
    int count;

    if (buffer == NULL || format == NULL)
        return 0;

    va_start(args, format);
    count = vsscanf(buffer, format, args);
    va_end(args);

    return count;
}

int vscanf(const char *format, va_list vlist) {
    return (PROFAN_FNI, 0);
}

int vfscanf(FILE *stream, const char *format, va_list vlist) {
    return (PROFAN_FNI, 0);
}

// vsscanf defined in funcs/vsscanf.c

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
    count = vsnprintf(buffer, -1, format, args);
    va_end(args);

    return count;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    int count;
    va_list args;

    va_start(args, format);
    count = vsnprintf(str, size, format, args);
    va_end(args);

    return count;
}

int vprintf(const char *format, va_list vlist) {
    int count = vsnprintf(g_printf_buffer, 0x4000, format, vlist);

    if ((int) fwrite(g_printf_buffer, 1, count, stdout) != count)
        return -1;

    return count;
}

int vfprintf(FILE *stream, const char *format, va_list vlist) {
    if (stream == NULL || (stream->mode & 0b11) == O_RDONLY)
        return -1;

    int count = vsnprintf(g_printf_buffer, 0x4000, format, vlist);

    if ((int) fwrite(g_printf_buffer, 1, count, stream) != count)
        return -1;

    return count;
}

int vsprintf(char *buffer, const char *format, va_list vlist) {
    return vsnprintf(buffer, -1, format, vlist);
}

// vsnprintf defined in funcs/vsnprintf.c

int feof(FILE *stream) {
    if (stream == NULL)
        return 1;

    return stream->eof;
}

int ferror(FILE *stream) {
    return (stream && stream->error) ? 1 : 0;
}

void rewind(FILE *stream) {
    fseek(stream, 0, SEEK_SET);
    clearerr(stream);
}

void perror(const char *s) {
    if (s != NULL) {
        fputs(s, stderr);
        fputs(": ", stderr);
    }
    fputs(strerror(errno), stderr);
    fputs("\n", stderr);
}

int remove(const char *name) {
    if (fu_is_dir(profan_path_resolve(name)))
        return rmdir(name);
    return unlink(name);
}

int rename(const char *old_filename, const char *new_filename) {
    char *tmp, *fullpath;
    char *new_entry;

    // check if the file exists
    fullpath = profan_path_join(profan_wd_path(), (char *) old_filename);
    profan_path_simplify(fullpath);
    uint32_t old_sid = fu_path_to_sid(SID_ROOT, fullpath);

    if (IS_SID_NULL(old_sid)) {
        errno = ENOENT;
        free(fullpath);
        return -1;
    }

    if (!fu_is_file(old_sid)) {
        errno = EISDIR;
        free(fullpath);
        return -1;
    }

    // get the parent directory sid
    profan_path_sep(fullpath, &tmp, NULL);
    uint32_t old_parent_sid = fu_path_to_sid(SID_ROOT, tmp);

    free(fullpath);
    free(tmp);

    // check if the new file exists
    fullpath = profan_path_join(profan_wd_path(), (char *) new_filename);
    profan_path_simplify(fullpath);
    uint32_t new_sid = fu_path_to_sid(SID_ROOT, fullpath);

    if (!IS_SID_NULL(new_sid)) {
        if (fu_is_dir(new_sid)) {
            errno = EISDIR;
            free(fullpath);
            return -1;
        }

        // remove the new file
        if (unlink(fullpath) < 0) {
            free(fullpath);
            return -1;
        }
    }

    // get the new parent directory sid
    profan_path_sep(fullpath, &tmp, &new_entry);
    uint32_t new_parent_sid = fu_path_to_sid(SID_ROOT, tmp);

    free(fullpath);
    free(tmp);

    if (IS_SID_NULL(new_parent_sid) || IS_SID_NULL(old_parent_sid)) {
        errno = ENOENT;
        free(new_entry);
        return -1;
    }

    // remove the old entry from the old parent
    if (fu_remove_from_dir(old_parent_sid, old_sid)) {
        free(new_entry);
        return -1;
    }

    // add the new entry to the new parent
    if (fu_add_to_dir(new_parent_sid, old_sid, new_entry)) {
        free(new_entry);
        return -1;
    }

    free(new_entry);
    return 0;
}

FILE *tmpfile(void) {
    char template[] = "/tmp/tmpfileXXXXXX";
    int fd = mkstemp(template);

    if (fd < 0)
        return NULL;

    return fdopen_mode(fd, O_RDWR | O_CREAT | O_TRUNC);
}

char *tmpnam(char *filename) {
    return (PROFAN_FNI, NULL);
}
