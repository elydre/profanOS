#include <kernel/filesystem.h>
#include <driver/keyboard.h>
#include <kernel/ramdisk.h>
#include <driver/serial.h>
#include <kernel/task.h>
#include <driver/ata.h>
#include <cpu/timer.h>
#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <gui/vgui.h>
#include <function.h>
#include <system.h>
#include <iolib.h>
#include <time.h>
#include <mem.h>

void init_watfunc() {
    *(int *)(WATFUNC_ADDR) = (int) wf_get_func_addr;
    *(int *)(WATDILY_ADDR) = (int) dily_get_func;
}

void unknown_func() {
    sys_error("Unknown syscall");
}

void up_string(char str[]) {
    UNUSED(str);
}

int wf_get_func_addr(int func_id) {
    switch (func_id) {
        // filesystem.h
        case 0:  return (int) fs_get_used_sectors;
        case 1:  return (int) fs_is_disk_full;
        case 2:  return (int) fs_make_dir;
        case 3:  return (int) fs_make_file;
        case 4:  return (int) fs_read_file;
        case 5:  return (int) fs_write_in_file;
        case 6:  return (int) fs_get_file_size;
        case 7:  return (int) fs_get_folder_size;
        case 8:  return (int) fs_declare_read_array;
        case 9:  return (int) fs_does_path_exists;
        case 10: return (int) fs_type_sector;
        case 11: return (int) fs_get_dir_content;
        case 12: return (int) fs_path_to_id;

        // mem.h
        case 13: return (int) mem_copy;
        case 14: return (int) mem_set;
        case 15: return (int) mem_alloc;
        case 16: return (int) mem_free_addr;
        case 17: return (int) free;
        case 18: return (int) calloc;
        case 19: return (int) malloc;
        case 20: return (int) realloc;
        case 21: return (int) mem_get_usage;
        case 22: return (int) mem_get_usable;
        case 61: return (int) mem_print;
        case 73: return (int) mem_get_alloc_count;
        case 74: return (int) mem_get_free_count;
        case 104: return (int) mem_get_phys_size;

        // time.h + rtc.h + timer.h
        case 42: return (int) time_gen_unix;
        case 43: return (int) sleep;
        case 44: return (int) ms_sleep;
        case 45: return (int) time_get_boot;
        case 66: return (int) time_jet_lag;
        case 71: return (int) time_calc_unix;
        case 70: return (int) time_get;
        case 72: return (int) timer_get_tick;

        // gnrtx.h
        case 46: return (int) clear_screen;
        case 49: return (int) ckprint_at;
        case 51: return (int) kprint_backspace;
        case 52: return (int) set_cursor_offset;
        case 53: return (int) get_cursor_offset;
        case 54: return (int) get_offset;
        case 55: return (int) gt_get_max_rows;
        case 56: return (int) gt_get_max_cols;
        case 76: return (int) cursor_blink;
        case 81: return (int) vesa_set_pixel;

        // keyboard.h
        case 57: return (int) kb_scancode_to_char;
        case 58: return (int) kb_get_scancode;
        case 91: return (int) kb_reset_history;
        case 92: return (int) kb_get_scfh;

        // functions.h
        case 59: return (int) pow;
        case 60: return (int) rand;

        // system.h
        case 62: return (int) sys_reboot;
        case 68: return (int) sys_shutdown;
        case 69: return (int) run_binary;
        case 75: return (int) run_ifexist;

        // ata.h
        case 65: return (int) ata_get_sectors_count;

        // task.h
        case 67: return (int) task_switch;
        case 84: return (int) task_get_alive;
        case 100: return (int) task_get_max;

        // vgui.h
        case 93: return (int) vgui_setup;
        case 94: return (int) vgui_exit;
        case 95: return (int) vgui_render;
        case 96: return (int) vgui_draw_rect;
        case 97: return (int) vgui_set_pixel;
        case 98: return (int) vgui_get_pixel;
        case 99: return (int) vgui_print;
        case 82: return (int) vgui_draw_line;
        case 83: return (int) vgui_clear;

        // serial.h
        case 102: return (int) serial_debug;
        case 103: return (int) serial_print;

        // ramdisk.h
        case 63: return (int) ramdisk_read_sector;
        case 64: return (int) ramdisk_write_sector;
        case 105: return (int) ramdisk_get_size;
        case 106: return (int) ramdisk_get_used;

        case 101: return (int) up_string;
        default: return (int) unknown_func;
    }
}
