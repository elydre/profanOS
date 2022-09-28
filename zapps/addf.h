#ifndef ADDF_H
#define ADDF_H


#define INIT_AF(addr) int (*get_func)(int id) = (int (*)(int)) addr

#define AF_fs_read_file() void (*fs_read_file)(char path[], uint32_t data[]) = (void (*)(char[], uint32_t[])) get_func(4)
#define AF_fs_declare_read_array() void * (*fs_declare_read_array)(char path[]) = (void * (*)(char[])) get_func(8)
#define AF_free() void (*free)(void * ptr) = (void (*)(void *)) get_func(17)
#define AF_malloc() void * (*malloc)(int size) = (void * (*)(int)) get_func(19)
#define AF_int_to_ascii() void (*int_to_ascii)(int n, char str[]) = (void (*)(int, char *)) get_func(23)
#define AF_append() void (*append)(char *str, char c) = (void (*)(char *, char)) get_func(29)
#define AF_str_cpy() void (*str_cpy)(char *dest, char *src) = (void (*)(char *, char *)) get_func(30)
#define AF_str_cmp() int (*str_cmp)(char str1[], char str2[]) = (int (*)(char[], char[])) get_func(31)
#define AF_str_start_split() void (*str_start_split)(char *str, char delim) = (void (*)(char *, char)) get_func(32)
#define AF_str_end_split() void (*str_end_split)(char *str, char delim) = (void (*)(char *, char)) get_func(33)
#define AF_mskprint() void (*mskprint)(int nb_args, ...) = (void (*)(int, ...)) get_func(37)
#define AF_fskprint() void (*fskprint)(char format[], ...) = (void (*)(char[], ...)) get_func(38)
#define AF_rainbow_print() void (*rainbow_print)(char msg[]) = (void (*)(char *)) get_func(39)
#define AF_input() void (*input)(char out_buffer[], int size, char color) = (void (*)(char *, int, char)) get_func(41)
#define AF_time_gen_unix() int (*time_gen_unix)() = (int (*)()) get_func(42)
#define AF_ms_sleep() void (*ms_sleep)(int) = (void (*)(int)) get_func(44)
#define AF_sleep() void (*sleep)(int) = (void (*)(int)) get_func(43)
#define AF_clear_screen() void (*clear_screen)() = (void (*)()) get_func(46)
#define AF_ckprint_at() void (*ckprint_at)(char *str, int x, int y, char color) = (void (*)(char *, int, int, char)) get_func(49)
#define AF_get_last_scancode() int (*get_last_scancode)() = (int (*)()) get_func(58)
#define AF_rand() int (*rand)() = (int (*)()) get_func(60)


#endif
