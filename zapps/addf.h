#ifndef ADDF_H
#define ADDF_H


#define INIT_AF(addr) int (*get_func)(int id) = (int (*)(int)) addr

#define AF_int_to_ascii() void (*int_to_ascii)(int n, char str[]) = (void (*)(int, char *)) get_func(23)
#define AF_append() void (*append)(char *str, char c) = (void (*)(char *, char)) get_func(29)
#define AF_rainbow_print() void (*rainbow_print)(char msg[]) = (void (*)(char *)) get_func(39)
#define AF_ms_sleep() void (*ms_sleep)(int) = (void (*)(int)) get_func(44)
#define AF_sleep() void (*sleep)(int) = (void (*)(int)) get_func(43)
#define AF_clear_screen() void (*clear_screen)() = (void (*)()) get_func(46)
#define AF_ckprint_at() void (*ckprint_at)(char *str, int x, int y, char color) = (void (*)(char *, int, int, char)) get_func(49)
#define AF_get_last_scancode() int (*get_last_scancode)() = (int (*)()) get_func(58)
#define AF_rand() int (*rand)() = (int (*)()) get_func(60)


#endif
