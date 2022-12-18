#ifndef SYSCALL_H
#define SYSCALL_H

#include <type.h>

#define WATFUNC_ADDR 0x199990

#define SERIAL_PORT_A 0x3F8
#define SERIAL_PORT_B 0x2F8

#define UNUSED(x) (void)(x)
#define ARYLEN(x) (int)(sizeof(x) / sizeof((x)[0]))

#define c_ckprint(message, color) c_ckprint_at(message, -1, -1, color)
#define c_kprint_rgb(message, color, bg_color) c_rgb_print_at(message, -1, -1, color, bg_color)
#define c_kprint(message) c_ckprint(message, c_white)

// nothing better than shit code art
#define hi_func_addr(id) ((int (*)(int)) *(int *)WATFUNC_ADDR)(id)

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
#define KB_TAB 15

#define KB_released_value 128
#define KB_released(key) (key + KB_released_value)

#define c_blue      0x09
#define c_green     0x0a
#define c_cyan      0x0b
#define c_red       0x0c
#define c_magenta   0x0d
#define c_yellow    0x0e
#define c_grey      0x07
#define c_white     0x0f

#define c_dblue     0x01
#define c_dgreen    0x02
#define c_dcyan     0x03
#define c_dred      0x04
#define c_dmagenta  0x05
#define c_dyellow   0x06
#define c_dgrey     0x08

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

typedef struct sprite_t {
    char *path;
    char *data;
    int x;
    int y;
    int size_x;
    int size_y;
} sprite_t;

#ifndef NULL
    #define NULL ((void *) 0)
#endif

#define ARYLEN(x) (int)(sizeof(x) / sizeof((x)[0]))

#define c_fs_get_used_sectors() ((uint32_t (*)(void)) hi_func_addr(0))()
#define c_fs_get_element_name(sector, name) ((void (*)(uint32_t, char *)) hi_func_addr(1))(sector, name)
#define c_fs_make_dir(path, folder_name) ((uint32_t (*)(char *, char *)) hi_func_addr(2))(path,folder_name)
#define c_fs_make_file(path, file_name) ((uint32_t (*)(char *, char *)) hi_func_addr(3))(path, file_name)
#define c_fs_read_file(path, data) ((void (*)(char *, uint8_t *)) hi_func_addr(4))(path, data)
#define c_fs_write_in_file(path, data, size) ((void (*)(char *, uint8_t *, uint32_t)) hi_func_addr(5))(path, data, size)
#define c_fs_get_file_size(path) ((uint32_t (*)(char *)) hi_func_addr(6))(path)
#define c_fs_get_dir_size(path) ((int (*)(char *)) hi_func_addr(7))(path)
#define c_fs_declare_read_array(path) ((void *(*)(char *)) hi_func_addr(8))(path)
#define c_fs_does_path_exists(path) ((int (*)(char *)) hi_func_addr(9))(path)
#define c_fs_get_sector_type(sector) ((int (*)(uint32_t)) hi_func_addr(10))(sector)
#define c_fs_get_dir_content(path, ids) ((void (*)(char *, uint32_t *)) hi_func_addr(11))(path, ids)
#define c_fs_path_to_id(path) ((uint32_t (*)(char *)) hi_func_addr(12))(path)
#define c_free(ptr) ((void (*)(void *)) hi_func_addr(17))(ptr)
#define c_calloc(size) ((void *(*)(int)) hi_func_addr(18))(size)
#define c_malloc(size) ((void *(*)(int)) hi_func_addr(19))(size)
#define c_realloc(ptr, size) ((void *(*)(void *, int)) hi_func_addr(20))(ptr, size)
#define c_time_gen_unix() ((int (*)(void)) hi_func_addr(42))()
#define c_ms_sleep(ms) ((void (*)(int)) hi_func_addr(44))(ms)
#define c_time_get_boot() ((int (*)(void)) hi_func_addr(45))()
#define c_clear_screen() ((void (*)(void)) hi_func_addr(46))()
#define c_ckprint_at(str, x, y, color) ((void (*)(char *, int, int, char)) hi_func_addr(49))(str, x, y, color)
#define c_kprint_backspace() ((void (*)(void)) hi_func_addr(51))()
#define c_set_cursor_offset(offset) ((void (*)(int)) hi_func_addr(52))(offset)
#define c_get_cursor_offset() ((int (*)(void)) hi_func_addr(53))()
#define c_gt_get_max_rows() ((int (*)(void)) hi_func_addr(55))()
#define c_gt_get_max_cols() ((int (*)(void)) hi_func_addr(56))()
#define c_kb_scancode_to_char(scancode, shift) ((char (*)(int, int)) hi_func_addr(57))(scancode, shift)
#define c_kb_get_scancode() ((int (*)(void)) hi_func_addr(58))()
#define c_pow(a, b) ((int (*)(int, int)) hi_func_addr(59))(a, b)
#define c_rand() ((int (*)(void)) hi_func_addr(60))()
#define c_mem_get_info(get_mode, get_arg) ((int (*)(int, int)) hi_func_addr(61))(get_mode, get_arg)
#define c_sys_reboot() ((void (*)(void)) hi_func_addr(62))()
#define c_ramdisk_read_sector(LBA, out) ((void (*)(uint32_t, uint32_t *)) hi_func_addr(63))(LBA, out)
#define c_ramdisk_write_sector(LBA, bytes) ((void (*)(uint32_t, uint32_t *)) hi_func_addr(64))(LBA, bytes)
#define c_fs_get_sector_count() ((uint32_t (*)(void)) hi_func_addr(65))()
#define c_time_jet_lag(time) ((void (*)(time_t *)) hi_func_addr(66))(time)
#define c_task_switch(target_pid) ((void (*)(int)) hi_func_addr(67))(target_pid)
#define c_sys_shutdown() ((void (*)(void)) hi_func_addr(68))()
#define c_run_binary(path, silence, nb_args, args) ((int (*)(char *, int, int, char **)) hi_func_addr(69))(path, silence, nb_args, args)
#define c_time_get(time) ((void (*)(time_t *)) hi_func_addr(70))(time)
#define c_time_calc_unix(time) ((int (*)(time_t *)) hi_func_addr(71))(time)
#define c_timer_get_tick() ((int (*)(void)) hi_func_addr(72))()
#define c_run_ifexist(path, argc, argv) ((int (*)(char *, int, char **)) hi_func_addr(75))(path, argc, argv)
#define c_cursor_blink(on) ((void (*)(int)) hi_func_addr(76))(on)
#define c_vesa_set_pixel(x, y, c) ((void (*)(int, int, uint32_t)) hi_func_addr(81))(x, y, c)
#define c_vgui_draw_line(x1, y1, x2, y2, c) ((void (*)(int, int, int, int, unsigned)) hi_func_addr(82))(x1, y1, x2, y2, c)
#define c_vgui_clear(color) ((void (*)(unsigned)) hi_func_addr(83))(color)
#define c_task_get_alive() ((int (*)(void)) hi_func_addr(84))()
#define c_lib2d_print_sprite(x, y, sprite) ((void (*)(int, int, sprite_t)) hi_func_addr(87))(x, y, sprite)
#define c_lib2d_free_sprite(sprite) ((void (*)(sprite_t)) hi_func_addr(88))(sprite)
#define c_lib2d_init_sprite(path) ((sprite_t (*)(char*)) hi_func_addr(89))(path)
#define c_kb_reset_history() ((void (*)(void)) hi_func_addr(91))()
#define c_kb_get_scfh() ((int (*)(void)) hi_func_addr(92))()
#define c_vgui_setup(refresh_all) ((void (*)(int)) hi_func_addr(93))(refresh_all)
#define c_vgui_exit() ((void (*)(void)) hi_func_addr(94))()
#define c_vgui_render() ((void (*)(void)) hi_func_addr(95))()
#define c_vgui_draw_rect(x, y, w, h, color) ((void (*)(int, int, int, int, unsigned)) hi_func_addr(96))(x, y, w, h, color)
#define c_vgui_set_pixel(x, y, color) ((void (*)(int, int, unsigned)) hi_func_addr(97))(x, y, color)
#define c_vgui_get_pixel(x, y) ((unsigned (*)(int, int)) hi_func_addr(98))(x, y)
#define c_vgui_print(x, y, msg, color) ((void (*)(int, int, char *, unsigned)) hi_func_addr(99))(x, y, msg, color)
#define c_task_get_max() ((int (*)(void)) hi_func_addr(100))()
#define c_serial_debug(source, message) ((void (*)(char *, char *)) hi_func_addr(102))(source, message)
#define c_serial_print(device, message) ((void (*)(int, char *)) hi_func_addr(103))(device, message)
#define c_ramdisk_get_size() ((uint32_t (*)(void)) hi_func_addr(105))()
#define c_ramdisk_get_used() ((uint32_t (*)(void)) hi_func_addr(106))()

#endif
