#ifndef ADDF_H
#define ADDF_H

#include <stdint.h>

typedef enum ScreenColor ScreenColor;
enum ScreenColor {
    // light colors
    c_blue = 0x09,
    c_green = 0x0a,
    c_cyan = 0x0b,
    c_red = 0x0c,
    c_magenta = 0x0d,
    c_yellow = 0x0e,
    c_grey = 0x07,
    c_white = 0x0f,
    
    // dark colors
    c_dblue = 0x01,
    c_dgreen = 0x02,
    c_dcyan = 0x03,
    c_dred = 0x04,
    c_dmagenta = 0x05,
    c_dyellow = 0x06,
    c_dgrey = 0x08,
};

typedef struct string_20_t {
    char name[20];
} string_20_t;

typedef struct {
    int seconds;
    int minutes;
    int hours;
    int day_of_week;
    int day_of_month;
    int month;
    int year;
    int full[6];
} time_t;

#define ARYLEN(x) (int)(sizeof(x) / sizeof((x)[0]))

#define INIT_AF(addr) int (*get_func)(int id) = (int (*)(int)) addr

#define AF_fs_get_used_sectors() uint32_t(*fs_get_used_sectors)(uint32_t disk_size) = (uint32_t (*)(uint32_t)) get_func(0)
#define AF_fs_make_dir() uint32_t (*fs_make_dir)(char path[], char folder_name[]) = (uint32_t (*)(char[], char[])) get_func(2)
#define AF_fs_make_file() uint32_t (*fs_make_file)(char path[], char file_name[]) = (uint32_t (*)(char[], char[])) get_func(3)
#define AF_fs_read_file() void (*fs_read_file)(char path[], uint32_t data[]) = (void (*)(char[], uint32_t[])) get_func(4)
#define AF_fs_write_in_file() void (*fs_write_in_file)(char path[], uint32_t data[], uint32_t data_size) = (void (*)(char[], uint32_t[], uint32_t)) get_func(5)
#define AF_fs_get_folder_size() int (*fs_get_folder_size)(char path[]) = (int (*)(char[])) get_func(7)
#define AF_fs_declare_read_array() void * (*fs_declare_read_array)(char path[]) = (void * (*)(char[])) get_func(8)
#define AF_fs_get_file_size() uint32_t (*fs_get_file_size)(char path[]) = (uint32_t (*)(char *)) get_func(6)
#define AF_fs_does_path_exists() int (*fs_does_path_exists)(char path[]) = (int (*)(char[])) get_func(9)
#define AF_fs_type_sector() int (*fs_type_sector)(uint32_t sector) = (int (*)(uint32_t)) get_func(10)
#define AF_fs_get_dir_content() void (*fs_get_dir_content)(uint32_t id, string_20_t list_name[], uint32_t liste_id[]) = (void (*)(uint32_t, string_20_t[], uint32_t[])) get_func(11)
#define AF_fs_path_to_id() uint32_t(*fs_path_to_id)(char input_path[], int silence) = (uint32_t(*)(char[], int)) get_func(12)
#define AF_mem_alloc() int (*mem_alloc)(int size) = (int (*)(int)) get_func(15)
#define AF_mem_free_addr() int (*mem_free_addr)(int addr) = (int (*)(int)) get_func(16)
#define AF_free() void (*free)(void * ptr) = (void (*)(void *)) get_func(17)
#define AF_calloc() void * (*calloc)(int size) = (void * (*)(int)) get_func(18)
#define AF_malloc() void * (*malloc)(int size) = (void * (*)(int)) get_func(19)
#define AF_mem_get_usage() int (*mem_get_usage)() = (int (*)()) get_func(21)
#define AF_mem_get_usable() int (*mem_get_usable)() = (int (*)()) get_func(22)
#define AF_int_to_ascii() void (*int_to_ascii)(int n, char str[]) = (void (*)(int, char *)) get_func(23)
#define AF_ascii_to_int() int (*ascii_to_int)(char str[]) = (int (*)(char[])) get_func(25)
#define AF_str_len() int (*str_len)(char s[]) = (int (*)(char *)) get_func(27)
#define AF_str_append() void (*str_append)(char *str, char c) = (void (*)(char *, char)) get_func(29)
#define AF_str_cpy() void (*str_cpy)(char *dest, char *src) = (void (*)(char *, char *)) get_func(30)
#define AF_str_cmp() int (*str_cmp)(char str1[], char str2[]) = (int (*)(char[], char[])) get_func(31)
#define AF_str_start_split() void (*str_start_split)(char *str, char delim) = (void (*)(char *, char)) get_func(32)
#define AF_str_end_split() void (*str_end_split)(char *str, char delim) = (void (*)(char *, char)) get_func(33)
#define AF_str_count() int (*str_count)(char str[], char thing) = (int (*)(char[], char)) get_func(35)
#define AF_str_cat() char* (*str_cat)(char s1[], const char s2[]) = (char* (*)(char[], const char[])) get_func(36)
#define AF_mskprint() void (*mskprint)(int nb_args, ...) = (void (*)(int, ...)) get_func(37)
#define AF_fskprint() void (*fskprint)(char format[], ...) = (void (*)(char[], ...)) get_func(38)
#define AF_rainbow_print() void (*rainbow_print)(char msg[]) = (void (*)(char *)) get_func(39)
#define AF_input_paste() void (*input_paste)(char out_buffer[], int size, char paste_buffer[], ScreenColor color) = (void (*)(char[], int, char[], ScreenColor)) get_func(40)
#define AF_input() void (*input)(char out_buffer[], int size, char color) = (void (*)(char *, int, char)) get_func(41)
#define AF_time_gen_unix() int (*time_gen_unix)() = (int (*)()) get_func(42)
#define AF_sleep() void (*sleep)(int) = (void (*)(int)) get_func(43)
#define AF_ms_sleep() void (*ms_sleep)(int) = (void (*)(int)) get_func(44)
#define AF_time_get_boot() int (*time_get_boot)() = (int (*)()) get_func(45)
#define AF_clear_screen() void (*clear_screen)() = (void (*)()) get_func(46)
#define AF_kprint() void (*kprint)(char *message) = (void (*)(char *)) get_func(47)
#define AF_ckprint() void (*ckprint)(char *message, char color) = (void (*)(char*, char)) get_func(48)
#define AF_ckprint_at() void (*ckprint_at)(char *str, int x, int y, char color) = (void (*)(char *, int, int, char)) get_func(49)
#define AF_kprint_backspace() void (*kprint_backspace)() = (void (*)()) get_func(51)
#define AF_kb_scancode_to_char() char (*kb_scancode_to_char)(int scancode, int shift) = (char (*)(int, int)) get_func(57)
#define AF_kb_get_scancode() int (*kb_get_scancode)() = (int (*)()) get_func(58)
#define AF_pow() int (*pow)(int a, int b) = (int (*)(int, int)) get_func(59)
#define AF_rand() int (*rand)() = (int (*)()) get_func(60)
#define AF_mem_print() void (*mem_print)() = (void (*)()) get_func(61)
#define AF_sys_reboot() void (*sys_reboot)() = (void (*)()) get_func(62)
#define AF_ata_read_sector() void (*ata_read_sector)(uint32_t LBA, uint32_t out[]) = (void (*)(uint32_t, uint32_t[])) get_func(63)
#define AF_ata_get_sectors_count() uint32_t (*ata_get_sectors_count)() = (uint32_t (*)()) get_func(65)
#define AF_timer_get_refresh_time() void (*timer_get_refresh_time)(int target[]) = (void (*)(int[])) get_func(66)
#define AF_yield() void (*yield)(int target_pid) = (void (*)(int)) get_func(67)
#define AF_sys_shutdown() void (*sys_shutdown)() = (void (*)()) get_func(68)
#define AF_sys_run_binary() int (*sys_run_binary)(char *fileName, int arg) = (int (*)(char *, int)) get_func(69)
#define AF_time_get() void (*time_get)(time_t *time) = (void (*)(time_t *)) get_func(70)
#define AF_time_calc_unix() int (*time_calc_unix)(time_t *time) = (int (*)(time_t *)) get_func(71)
#define AF_timer_get_tick() int (*timer_get_tick)() = (int (*)()) get_func(72)
#define AF_mem_get_alloc_count() int (*mem_get_alloc_count)() = (int (*)()) get_func(73)
#define AF_mem_get_free_count() int (*mem_get_free_count)() = (int (*)()) get_func(74)
#define AF_task_print() void (*task_print)() = (void (*)()) get_func(75)
#define AF_cursor_blink() void (*cursor_blink)(int on) = (void (*)(int)) get_func(76)

#endif
