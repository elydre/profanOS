#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#define FUNC_ADDR_SAVE 0x199990

#define UNUSED(x) (void)(x)
#define ARYLEN(x) (int)(sizeof(x) / sizeof((x)[0]))

// nothing better than shit code art
#define hi_func_addr(id) ((int (*)(int)) *(int *)FUNC_ADDR_SAVE)(id)

#define KB_A 16
#define KB_Z 17
#define KB_E 18
#define KB_R 19
#define KB_T 20
#define KB_Y 21
#define KB_U 22
#define KB_I 23
#define KB_O 24
#define KB_P 25
#define KB_Q 30
#define KB_S 31
#define KB_D 32
#define KB_F 33
#define KB_G 34
#define KB_H 35
#define KB_J 36
#define KB_K 37
#define KB_L 38
#define KB_M 39
#define KB_N 49
#define KB_W 44
#define KB_X 45
#define KB_C 46
#define KB_V 47
#define KB_B 48

#define KB_ESC 1
#define KB_ALT 56
#define KB_CTRL 29
#define KB_SHIFT 42
#define KB_MAJ 58

#define KB_released_value 128
#define KB_released(key) key+KB_released_value

#ifdef __cplusplus // si on est en c++ les conditions sont différentes, il n'y a pas besoin de définir screen_color_t
extern "C" {
    enum screen_color_t {
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

    typedef struct {
        char *path;;
        char *data;
        int x;
        int y;
        int size_x;
        int size_y;
    } Sprite_t;
}
#else
    typedef enum screen_color_t screen_color_t;
    enum screen_color_t {
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

    typedef struct Sprite_t {
        char *path;
        char *data;
        int x;
        int y;
        int size_x;
        int size_y;
    } Sprite_t;
#endif

#ifndef NULL
    #ifdef __cplusplus
        #define NULL 0
    #else
        #define NULL ((void *)0)
    #endif
#endif

#define ARYLEN(x) (int)(sizeof(x) / sizeof((x)[0]))

#define c_fs_get_used_sectors(disk_size) ((uint32_t (*)(uint32_t)) hi_func_addr(0))(disk_size)
#define c_fs_make_dir(path,folder_name) ((uint32_t (*)(char *, char *)) hi_func_addr(2))(path,folder_name)
#define c_fs_make_file(path, file_name) ((uint32_t (*)(char *, char *)) hi_func_addr(3))(path, file_name)
#define c_fs_read_file(path, data) ((void (*)(char *, uint32_t *)) hi_func_addr(4))(path, data)
#define c_fs_write_in_file(path, data, data_size) ((void (*)(char *, uint32_t *, uint32_t)) hi_func_addr(5))(path, data, data_size)
#define c_fs_get_folder_size(path) ((int (*)(char *)) hi_func_addr(7))(path)
#define c_fs_declare_read_array(path) ((void * (*)(char *)) hi_func_addr(8))(path)
#define c_fs_get_file_size(path) ((uint32_t (*)(char *)) hi_func_addr(6))(path)
#define c_fs_does_path_exists(path) ((int (*)(char *)) hi_func_addr(9))(path)
#define c_fs_type_sector(sector) ((int (*)(uint32_t)) hi_func_addr(10))(sector)
#define c_fs_get_dir_content(id, list_name, liste_id) ((void (*)(uint32_t, string_20_t *, uint32_t *)) hi_func_addr(11))(id, list_name, liste_id)
#define c_fs_path_to_id(input_path, silence) ((uint32_t(*)(char *, int)) hi_func_addr(12))(input_path, silence)
#define c_mem_alloc(size) ((int (*)(int)) hi_func_addr(15))(size)
#define c_mem_free_addr(addr) ((int (*)(int)) hi_func_addr(16))(addr)
#define c_free(ptr) ((void (*)(void *)) hi_func_addr(17))(ptr)
#define c_calloc(size) ((void * (*)(int)) hi_func_addr(18))(size)
#define c_malloc(size) ((void * (*)(int)) hi_func_addr(19))(size)
#define c_mem_get_usage() ((int (*)(void)) hi_func_addr(21))()
#define c_mem_get_usable() ((int (*)(void)) hi_func_addr(22))()
#define c_int_to_ascii(n, str) ((void (*)(int, char *)) hi_func_addr(23))(n, str)
#define c_ascii_to_int(str) ((int (*)(char *)) hi_func_addr(25))(str)
#define c_str_len(s) ((int (*)(char *)) hi_func_addr(27))(s)
#define c_str_append(str, c) ((void (*)(char *, char)) hi_func_addr(29))(str, c)
#define c_str_cpy(dest, src) ((void (*)(char *, char *)) hi_func_addr(30))(dest, src)
#define c_str_cmp(str1, str2) ((int (*)(char *, char *)) hi_func_addr(31))(str1, str2)
#define c_str_start_split(str, delim) ((void (*)(char *, char)) hi_func_addr(32))(str, delim)
#define c_str_end_split(str, delim) ((void (*)(char *, char)) hi_func_addr(33))(str, delim)
#define c_str_count(str, thing) ((int (*)(char *, char)) hi_func_addr(35))(str, thing)
#define c_str_cat(s1, s2) ((void (*)(char *, char *)) hi_func_addr(36))(s1, s2)
#define c_mskprint(...) ((void (*)(int, ...)) hi_func_addr(37))(__VA_ARGS__)
#define c_fskprint(...) ((void (*)(char *, ...)) hi_func_addr(38))(__VA_ARGS__)
#define c_rainbow_print(msg) ((void (*)(char *)) hi_func_addr(39))(msg)
#define c_input_wh(out_buffer, size, color, history, history_size) ((void (*)(char *, int, screen_color_t, char **, int)) hi_func_addr(40))(out_buffer, size, color, history, history_size)
#define c_input(out_buffer, size, color) ((void (*)(char *, int, char)) hi_func_addr(41))(out_buffer, size, color)
#define c_time_gen_unix() ((int (*)(void)) hi_func_addr(42))()
#define c_sleep(seconds) ((void (*)(int)) hi_func_addr(43))(seconds)
#define c_ms_sleep(ms) ((void (*)(int)) hi_func_addr(44))(ms)
#define c_time_get_boot() ((int (*)(void)) hi_func_addr(45))()
#define c_clear_screen() ((void (*)(void)) hi_func_addr(46))()
#define c_kprint(message) ((void (*)(char *)) hi_func_addr(47))(message)
#define c_ckprint(message, color) ((void (*)(char*, char)) hi_func_addr(48))(message, color)
#define c_ckprint_at(str, x, y, color) ((void (*)(char *, int, int, char)) hi_func_addr(49))(str, x, y, color)
#define c_kprint_backspace() ((void (*)(void)) hi_func_addr(51))()
#define c_kb_scancode_to_char(scancode, shift) ((char (*)(int, int)) hi_func_addr(57))(scancode, shift)
#define c_kb_get_scancode() ((int (*)(void)) hi_func_addr(58))()
#define c_pow(a, b) ((int (*)(int, int)) hi_func_addr(59))(a, b)
#define c_rand() ((int (*)(void)) hi_func_addr(60))()
#define c_mem_print() ((void (*)(void)) hi_func_addr(61))()
#define c_sys_reboot() ((void (*)(void)) hi_func_addr(62))()
#define c_ata_read_sector(LBA, out) ((void (*)(uint32_t, uint32_t *)) hi_func_addr(63))(LBA, out)
#define c_ata_get_sectors_count() ((uint32_t (*)(void)) hi_func_addr(65))()
#define c_time_jet_lag(time) ((void (*)(time_t *)) hi_func_addr(66))(time)
#define c_yield(target_pid) ((void (*)(int)) hi_func_addr(67))(target_pid)
#define c_sys_shutdown() ((void (*)(void)) hi_func_addr(68))()
#define c_sys_run_binary(fileName, arg, silence) ((int (*)(char *, int, int)) hi_func_addr(69))(fileName, arg, silence)
#define c_time_get(time) ((void (*)(time_t *)) hi_func_addr(70))(time)
#define c_time_calc_unix(time) ((int (*)(time_t *)) hi_func_addr(71))(time)
#define c_timer_get_tick() ((int (*)(void)) hi_func_addr(72))()
#define c_mem_get_alloc_count() ((int (*)(void)) hi_func_addr(73))()
#define c_mem_get_free_count() ((int (*)(void)) hi_func_addr(74))()
#define c_task_print() ((void (*)(void)) hi_func_addr(75))()
#define c_cursor_blink(on) ((void (*)(int)) hi_func_addr(76))(on)
#define c_vga_320_mode() ((void (*)(void)) hi_func_addr(77))()
#define c_vga_640_mode() ((void (*)(void)) hi_func_addr(78))()
#define c_vga_text_mode() ((void (*)(void)) hi_func_addr(79))()
#define c_vga_clear_screen() ((void (*)(void)) hi_func_addr(80))()
#define c_vga_put_pixel(x, y, c) ((void (*)(unsigned, unsigned, unsigned)) hi_func_addr(81))(x, y, c)
#define c_vga_print(x, y, msg, big, color) ((void (*)(int, int, char *, int, unsigned)) hi_func_addr(82))(x, y, msg, big, color)
#define c_vga_draw_line(x1, y1, x2, y2, color) ((void (*)(int, int, int, int, unsigned)) hi_func_addr(83))(x1, y1, x2, y2, color)
#define c_vga_draw_rect(x, y, w, h, color) ((void (*)(int, int, int, int, unsigned)) hi_func_addr(84))(x, y, w, h, color)
#define c_vga_get_width() ((int (*)(void)) hi_func_addr(85))()
#define c_vga_get_height() ((int (*)(void)) hi_func_addr(86))()
#define c_lib2d_print_sprite(x, y, sprite) ((void (*)(int, int, Sprite_t)) hi_func_addr(87))(x, y, sprite)
#define c_lib2d_free_sprite(sprite) ((void (*)(Sprite_t)) hi_func_addr(88))(sprite)
#define c_lib2d_init_sprite(path) ((Sprite_t (*)(char*)) hi_func_addr(89))(path)
#define c_str_delchar(s, c) ((void (*)(char*, char)) hi_func_addr(90))(s, c)
#define c_kb_reset_history() ((void (*)(void)) hi_func_addr(91))()
#define c_kb_get_scfh() ((int (*)(void)) hi_func_addr(92))()
#define c_vgui_setup(refresh_all) ((void (*)(int)) hi_func_addr(93))(refresh_all)
#define c_vgui_exit() ((void (*)(void)) hi_func_addr(94))()
#define c_vgui_render() ((void (*)(void)) hi_func_addr(95))()
#define c_vgui_draw_rect(x, y, w, h, color) ((void (*)(int, int, int, int, unsigned)) hi_func_addr(96))(x, y, w, h, color)
#define c_vgui_set_pixel(x, y, color) ((void (*)(int, int, unsigned)) hi_func_addr(97))(x, y, color)
#define c_vgui_get_pixel(x, y) ((unsigned (*)(int, int)) hi_func_addr(98))(x, y)

#endif
