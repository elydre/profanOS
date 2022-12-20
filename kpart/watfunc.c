#include <kernel/filesystem.h>
#include <kernel/snowflake.h>
#include <driver/keyboard.h>
#include <kernel/ramdisk.h>
#include <driver/serial.h>
#include <kernel/task.h>
#include <driver/ata.h>
#include <driver/rtc.h>
#include <cpu/timer.h>
#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <gui/vgui.h>
#include <system.h>

int wf_get_func_addr(int func_id);

void init_watfunc() {
    *(int *)(WATFUNC_ADDR) = (int) wf_get_func_addr;
    *(int *)(WATDILY_ADDR) = (int) dily_get_func;
}

void unknown_func() {
    sys_error("Unknown syscall");
}

int wf_get_func_addr(int func_id) {
    switch (func_id) {
        // filesystem.h

        case 0:  return (int) fs_get_used_sectors;
        case 1: return (int) fs_get_sector_count;
        case 2:  return (int) fs_get_element_name;
        case 3:  return (int) fs_make_dir;
        case 4:  return (int) fs_make_file;
        case 5:  return (int) fs_read_file;
        case 6:  return (int) fs_write_in_file;
        case 7:  return (int) fs_get_file_size;
        case 8:  return (int) fs_get_dir_size;
        case 9:  return (int) fs_declare_read_array;
        case 10:  return (int) fs_does_path_exists;
        case 11: return (int) fs_get_sector_type;
        case 12: return (int) fs_get_dir_content;
        case 13: return (int) fs_path_to_id;

        // mem.h
        case 14: return (int) mem_get_alloc_size;
        case 15: return (int) mem_alloc;
        case 16: return (int) mem_free_addr;
        case 17: return (int) mem_get_info;
        case 43: return (int) mem_free_all; // not mem.h, but related to snowflake

        // time.h + rtc.h + timer.h
        case 18: return (int) time_get;
        case 19: return (int) timer_get_tick;

        // gnrtx.h
        case 20: return (int) font_get;
        case 21: return (int) clear_screen;
        case 22: return (int) ckprint_at;
        case 23: return (int) kprint_backspace;
        case 24: return (int) set_cursor_offset;
        case 25: return (int) get_cursor_offset;
        case 26: return (int) gt_get_max_rows;
        case 27: return (int) gt_get_max_cols;
        case 28: return (int) cursor_blink;
        case 29: return (int) vesa_set_pixel;

        // keyboard.h
        case 30: return (int) kb_scancode_to_char;
        case 31: return (int) kb_get_scancode;
        case 32: return (int) kb_reset_history;
        case 33: return (int) kb_get_scfh;

        // system.h
        case 34: return (int) sys_reboot;
        case 35: return (int) sys_shutdown;
        case 36: return (int) run_ifexist;

        // task.h
        case 37: return (int) task_switch;
        case 38: return (int) task_get_alive;
        case 39: return (int) task_get_max;
        case 44: return (int) task_get_current_pid;
        case 45: return (int) task_kill_task_switch;
        case 46: return (int) task_get_next_pid;

        // serial.h
        case 40: return (int) serial_print;

        // ramdisk.h
        case 41: return (int) ramdisk_get_size;
        case 42: return (int) ramdisk_get_used;

        default: return (int) unknown_func;
    }
}
