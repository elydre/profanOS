#include <syscall.h>
#include <i_iolib.h>
#include <i_string.h>
#include <type.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#define STDIO_C
#include <stdio.h>

void init_func();
int printf(const char *restrict format, ... );
int fflush(FILE *stream);
int fclose(FILE *stream);

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

int printf(const char *restrict format, ... ) {
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
    return 0;
}

int sprintf( char *restrict buffer, const char *restrict format, ... ) {
    fsprint("sprintf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int snprintf( char *restrict buffer, size_t bufsz, const char *restrict format, ... ) {
    fsprint("snprintf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
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

int vsnprintf( char *restrict buffer, size_t bufsz, const char *restrict format, va_list vlist ) {
    fsprint("vsnprintf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
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
