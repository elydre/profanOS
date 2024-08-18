/*****************************************************************************\
|   === syscall.c : 2024 ===                                                  |
|                                                                             |
|    Syscall handler                                               .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

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

#include <kernel/syscall.h>
#include <minilib.h>
#include <system.h>

void *SYSCALL_ARRAY[] = {
    // butterfly.h
    fs_get_main,
    fs_cnt_set_size,
    fs_cnt_get_size,
    fs_cnt_delete,
    fs_cnt_read,
    fs_cnt_write,
    fs_cnt_init,
    fs_cnt_meta,

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

    // scubasuit.h
    scuba_call_generate,
    scuba_call_map,
    scuba_call_unmap,
    scuba_call_phys
};

#define SYSCALL_COUNT 48

void syscall_handler(registers_t *r) {
    uint32_t syscall_id = r->eax;

    if (syscall_id >= SYSCALL_COUNT) {
        kprintf("syscall %d not found\n", syscall_id);
        return;
    }

    kprintf("syscall %d\n", syscall_id);

    uint32_t (*func)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) = SYSCALL_ARRAY[syscall_id];
    r->eax = func(r->ebx, r->ecx, r->edx, r->esi, r->edi);

    kprintf("syscall %d done\n", syscall_id);

    return;
}
