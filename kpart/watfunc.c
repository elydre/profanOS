#include <kernel/filesystem.h>
#include <kernel/snowflake.h>
#include <driver/keyboard.h>
#include <kernel/process.h>
#include <kernel/ramdisk.h>
#include <driver/serial.h>
#include <driver/mouse.h>
#include <driver/rtc.h>
#include <cpu/timer.h>
#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <minilib.h>
#include <system.h>

int wf_get_func_addr(int func_id);

int init_watfunc() {
    *(int *)(WATFUNC_ADDR) = (int) wf_get_func_addr;
    *(int *)(WATDILY_ADDR) = (int) dily_get_func;
    return 0;
}

void unknown_func() {
    sys_error("Unknown syscall");
}

void *SYSCALL_ARRAY[] = {
    // filesystem.h
    fs_get_used_sectors,
    fs_get_sector_count,
    fs_get_element_name,
    fs_make_dir,
    fs_make_file,
    fs_read_file,
    fs_write_in_file,
    fs_get_file_size,
    fs_get_dir_size,
    fs_declare_read_array,
    fs_does_path_exists,
    fs_get_sector_type,
    fs_get_dir_content,
    fs_path_to_id,
    fs_delete_file,
    fs_delete_dir,

    // snowflake.h
    mem_get_alloc_size,
    mem_alloc,
    mem_free_addr,
    mem_get_info,
    mem_free_all,

    // rtc.h + timer.h
    time_get,
    timer_get_ms,

    // gnrtx.h
    font_get,
    clear_screen,
    ckprint_at,
    kprint_backspace,
    set_cursor_offset,
    get_cursor_offset,
    gt_get_max_rows,
    gt_get_max_cols,
    cursor_blink,
    vesa_set_pixel,
    vesa_get_width,
    vesa_get_height,

    // keyboard.h
    kb_scancode_to_char,
    kb_get_scancode,
    kb_reset_history,
    kb_get_scfh,

    // system.h
    sys_reboot,
    sys_shutdown,
    run_ifexist_full,
    sys_kinfo,

    // serial.h
    serial_print,

    // mouse.h
    mouse_call,

    // ramdisk.h
    ramdisk_get_info,

    // process.h
    process_set_sheduler,
    process_create,
    process_sleep,
    process_wakeup,
    process_kill,
    process_get_pid,
    process_get_ppid,
    process_generate_pid_list,
    process_get_name,
    process_get_state,
    process_get_run_time,

    // minilib.h
    exit_pid,

    // system.h (dily)
    dily_unload,
    dily_load,
};

int wf_get_func_addr(int func_id) {
    if (func_id < 0 || func_id >= ARYLEN(SYSCALL_ARRAY)) {
        return (int) unknown_func;
    }
    return (int) SYSCALL_ARRAY[func_id];
}
