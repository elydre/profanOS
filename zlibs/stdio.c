#include <syscall.h>
#include <i_iolib.h>
#include <i_string.h>
#include <type.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void init_func();

int main() {
    init_func();
    return 0;
}

void init_func() {
    c_kprint("Init of the stdio lib !\n");
}

void clearerr(FILE *stream) {
    fsprint("clearerr not implemented yet, WHY DO YOU USE IT ?\n");
}

FILE *fopen( const char *restrict filename, const char *restrict mode ) {
    fsprint("fopen not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

errno_t fopen_s(FILE *restrict *restrict streamptr, const char *restrict filename, const char *restrict mode ) {
    fsprint("fopen_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

FILE *freopen(const char *restrict filename, const char *restrict mode, FILE *restrict stream) {
    fsprint("freopen not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

errno_t freopen_s( FILE *restrict *restrict newstreamptr, const char *restrict filename, const char *restrict mode, FILE *restrict stream ) {
    fsprint("freopen_s not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fclose(FILE *stream) {
    fsprint("fclose not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fflush(FILE *stream) {
    fsprint("fflush not implemented yet, WHY DO YOU USE IT ?\n");
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
    fsprint("fread not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

size_t fwrite(const void *restrict buffer, size_t size, size_t count, FILE *restrict stream) {
    fsprint("fwrite not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
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

int printf( const char *restrict format, ... ) {
    fsprint("printf not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fprintf( FILE *restrict stream, const char *restrict format, ... ) {
    fsprint("fprintf not implemented yet, WHY DO YOU USE IT ?\n");
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

FILE *tmpfile(void) {
    fsprint("tmpfile not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

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


