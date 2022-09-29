#include <driver/keyboard.h>
#include <driver/screen.h>
#include <task.h>
#include <driver/ata.h>
#include <filesystem.h>
#include <cpu/timer.h>
#include <string.h>
#include <system.h>
#include <iolib.h>
#include <time.h>
#include <mem.h>

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

        // string.h
        case 23: return (int) int_to_ascii;
        case 24: return (int) hex_to_ascii;
        case 25: return (int) ascii_to_int;
        case 26: return (int) str_reverse;
        case 27: return (int) str_len;
        case 28: return (int) str_backspace;
        case 29: return (int) str_append;
        case 30: return (int) str_cpy;
        case 31: return (int) str_cmp;
        case 32: return (int) str_start_split;
        case 33: return (int) str_end_split;
        case 34: return (int) str_is_in;
        case 35: return (int) str_count;
        case 36: return (int) str_cat;

        // iolib.h
        case 37: return (int) mskprint;
        case 38: return (int) fskprint;
        case 39: return (int) rainbow_print;
        case 40: return (int) input_paste;
        case 41: return (int) input;

        // time.h + rtc.h + timer.h
        case 42: return (int) time_gen_unix;
        case 43: return (int) sleep;
        case 44: return (int) ms_sleep;
        case 45: return (int) time_get_boot;
        case 66: return (int) timer_get_refresh_time;
        case 71: return (int) time_calc_unix;
        case 70: return (int) time_get;
        case 72: return (int) timer_get_tick;

        // screen.h
        case 46: return (int) clear_screen;
        case 47: return (int) kprint;
        case 48: return (int) ckprint;
        case 49: return (int) ckprint_at;
        case 50: return (int) print_char;
        case 51: return (int) kprint_backspace;
        case 52: return (int) set_cursor_offset;
        case 53: return (int) get_cursor_offset;
        case 54: return (int) get_offset;
        case 55: return (int) get_offset_row;
        case 56: return (int) get_offset_col;

        // keyboard.h
        case 57: return (int) kb_scancode_to_char;
        case 58: return (int) kb_get_scancode;

        // functions.h
        case 59: return (int) pow;
        case 60: return (int) rand;

        // system.h
        case 62: return (int) sys_reboot;
        case 68: return (int) sys_shutdown;
        case 69: return (int) sys_run_binary;

        // ata driver
        case 63: return (int) ata_read_sector;
        case 64: return (int) ata_write_sector;
        case 65: return (int) ata_get_sectors_count;

        // task.h
        case 67: return (int) yield;
        case 75: return (int) task_print;

        default: return 0;
    }
}
