#ifndef SYSCALL_H
#define SYSCALL_H

#include <type.h>

#define WATFUNC_ADDR 0x1ffff7

#define SERIAL_PORT_A 0x3F8
#define SERIAL_PORT_B 0x2F8

#define UNUSED(x) (void)(x)
#define ARYLEN(x) (int)(sizeof(x) / sizeof((x)[0]))

#define c_kcprint(message, color) c_kcnprint(message, -1, color)
#define c_kprint(message) c_kcprint(message, 0x0F)

#define c_run_ifexist(path, argc, argv) \
        c_run_ifexist_full((runtime_args_t){path, (sid_t){0, 0}, \
        argc, argv, 0, 0, 0, 1}, NULL)

#define c_fs_cnt_read(fs, head_sid, buf, offset, size) c_fs_cnt_rw(fs, head_sid, buf, offset, size, 1)
#define c_fs_cnt_write(fs, head_sid, buf, offset, size) c_fs_cnt_rw(fs, head_sid, buf, offset, size, 0)

#define c_vesa_get_width()   c_vesa_get_info(0)
#define c_vesa_get_height()  c_vesa_get_info(1)
#define c_vesa_get_pitch()   c_vesa_get_info(2)
#define c_vesa_get_fb() (void *) c_vesa_get_info(3)
#define c_vesa_does_enable() c_vesa_get_info(4)

// nothing better than shit code art
#define hi_func_addr(id) ((uint32_t (*)(uint32_t)) *(uint32_t *) WATFUNC_ADDR)(id)

#define c_fs_get_main ((filesys_t *(*)(void)) hi_func_addr(0))
#define c_fs_cnt_set_size ((int (*)(filesys_t *, sid_t, uint32_t)) hi_func_addr(1))
#define c_fs_cnt_get_size ((uint32_t (*)(filesys_t *, sid_t)) hi_func_addr(2))
#define c_fs_cnt_delete ((int (*)(filesys_t *, sid_t)) hi_func_addr(3))
#define c_fs_cnt_rw ((int (*)(filesys_t *, sid_t, void *, uint32_t, uint32_t, int)) hi_func_addr(4))
#define c_fs_cnt_init ((sid_t (*)(filesys_t *, uint32_t, char *)) hi_func_addr(5))
#define c_fs_cnt_get_meta ((char *(*)(filesys_t *, sid_t)) hi_func_addr(6))
#define c_fs_cnt_change_meta ((void (*)(filesys_t *, sid_t, char *)) hi_func_addr(7))

#define c_mem_get_alloc_size ((uint32_t (*)(uint32_t)) hi_func_addr(8))
#define c_mem_alloc ((uint32_t (*)(uint32_t, uint32_t, int)) hi_func_addr(9))
#define c_mem_free_addr ((int (*)(uint32_t)) hi_func_addr(10))
#define c_mem_get_info ((int (*)(int, int)) hi_func_addr(11))
#define c_mem_free_all ((int (*)(int)) hi_func_addr(12))

#define c_time_get ((void (*)(tm_t *)) hi_func_addr(13))
#define c_timer_get_ms ((uint32_t (*)(void)) hi_func_addr(14))

#define c_font_get ((uint8_t *(*)(int)) hi_func_addr(15))
#define c_kcnprint ((void (*)(char *, int, char)) hi_func_addr(16))
#define c_get_cursor_offset ((int (*)(void)) hi_func_addr(17))
#define c_vesa_set_pixel ((void (*)(int, int, uint32_t)) hi_func_addr(18))
#define c_vesa_get_info ((int (*)(int)) hi_func_addr(19))

#define c_sys_set_reporter ((void (*)(int (*)(char *))) hi_func_addr(20))

#define c_kb_scancode_to_char ((char (*)(int, int)) hi_func_addr(21))
#define c_kb_get_scancode ((int (*)(void)) hi_func_addr(22))
#define c_kb_get_scfh ((int (*)(void)) hi_func_addr(23))

#define c_sys_reboot ((void (*)(void)) hi_func_addr(24))
#define c_sys_shutdown ((void (*)(void)) hi_func_addr(25))
#define c_run_ifexist_full ((int (*)(runtime_args_t, int *)) hi_func_addr(26))
#define c_sys_kinfo ((char *(*)(void)) hi_func_addr(27))

#define c_serial_print ((void (*)(int, char *)) hi_func_addr(28))
#define c_mouse_call ((int (*)(int, int)) hi_func_addr(29))

#define c_process_set_sheduler ((void (*)(int)) hi_func_addr(30))
#define c_process_create ((int (*)(void (*func)(), int, char *)) hi_func_addr(31))
#define c_process_sleep ((void (*)(uint32_t, uint32_t)) hi_func_addr(32))
#define c_process_wakeup ((void (*)(uint32_t)) hi_func_addr(33))
#define c_process_kill ((void (*)(uint32_t)) hi_func_addr(34))
#define c_process_get_pid ((int (*)(void)) hi_func_addr(35))
#define c_process_get_ppid ((int (*)(uint32_t)) hi_func_addr(36))
#define c_process_generate_pid_list ((int (*)(uint32_t *, int)) hi_func_addr(37))
#define c_process_get_name ((int (*)(uint32_t, char *)) hi_func_addr(38))
#define c_process_get_state ((int (*)(uint32_t)) hi_func_addr(39))
#define c_process_get_run_time ((uint32_t (*)(uint32_t)) hi_func_addr(40))

#define c_exit_pid ((int (*)(int, int)) hi_func_addr(41))

#define c_dily_unload ((int (*)(uint32_t)) hi_func_addr(42))
#define c_dily_load ((int (*)(char *, uint32_t)) hi_func_addr(43))

#endif
