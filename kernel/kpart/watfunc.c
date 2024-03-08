#include <drivers/keyboard.h>
#include <kernel/butterfly.h>
#include <kernel/snowflake.h>
#include <kernel/process.h>
#include <drivers/serial.h>
#include <drivers/mouse.h>
#include <drivers/rtc.h>
#include <cpu/timer.h>
#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <minilib.h>
#include <system.h>

uint32_t wf_get_func_addr(uint32_t func_id);

int init_watfunc(void) {
    *(int *)(WATFUNC_ADDR) = (int) wf_get_func_addr;
    *(int *)(WATDILY_ADDR) = (int) dily_get_func;
    return 0;
}

void unknown_func(void) {
    sys_warning("Trying to call an unknown syscall");
}

void *SYSCALL_ARRAY[] = {
    // butterfly.h
    fs_get_main,
    fs_cnt_set_size,
    fs_cnt_get_size,
    fs_cnt_delete,
    fs_cnt_rw,
    fs_cnt_init,
    fs_cnt_get_meta,
    fs_cnt_change_meta,

    // snowflake.h
    mem_get_alloc_size,
    mem_alloc,
    mem_free_addr,
    mem_get_info,
    mem_free_all,

    // rtc.h + timer.h
    time_get,
    timer_get_ms,

    // gnrtx.h + vesa.h
    font_get,
    kcnprint,
    get_cursor_offset,
    vesa_set_pixel,
    vesa_get_info,
    sys_set_reporter,

    // keyboard.h
    kb_scancode_to_char,
    kb_get_scancode,
    kb_get_scfh,

    // system.h
    sys_reboot,
    sys_shutdown,
    binary_exec,
    sys_kinfo,

    // serial.h
    serial_read,
    serial_write,

    // mouse.h
    mouse_call,

    // process.h
    process_set_scheduler,
    process_create,
    process_fork,
    process_sleep,
    process_handover,
    process_wakeup,
    process_kill,
    process_get_pid,
    process_generate_pid_list,
    process_get_info,

    // runtime.h
    force_exit_pid,

    // system.h
    dily_unload,
    dily_load,
};

uint32_t wf_get_func_addr(uint32_t func_id) {
    if (func_id >= ARYLEN(SYSCALL_ARRAY)) {
        return (uint32_t) unknown_func;
    }
    return (uint32_t) SYSCALL_ARRAY[func_id];
}
