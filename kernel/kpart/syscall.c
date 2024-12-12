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

#include <minilib.h>
#include <system.h>

void *SYSCALL_ARRAY[] = {
    // butterfly.h
    fs_get_main,        // 0
    fs_cnt_read,        // 1
    fs_cnt_write,       // 2
    fs_cnt_set_size,    // 3
    fs_cnt_get_size,    // 4
    fs_cnt_delete,      // 5
    fs_cnt_init,        // 6
    fs_cnt_meta,        // 7

    // snowflake.h
    mem_alloc,          // 8
    mem_free_addr,      // 9
    mem_free_all,       // 10
    mem_get_alloc_size, // 11
    mem_get_info,       // 12

    // rtc.h + timer.h
    timer_get_ms,       // 13
    time_get,           // 14

    // gnrtx.h + vesa.h
    font_get,           // 15
    kcnprint,           // 16
    cursor_get_offset,  // 17
    vesa_get_info,      // 18

    // serial.h
    serial_read,        // 19
    serial_write,       // 20

    // keyboard.h + mouse.h
    kb_sc_to_char,      // 21
    kb_get_scfh,        // 22
    mouse_call,         // 23

    // system.h
    sys_set_reporter,   // 24
    sys_power,          // 25
    sys_kinfo,          // 26

    elf_exec,        // 27

    // process.h + runtime.h
    process_auto_schedule, // 28
    process_create,     // 29
    process_fork,       // 30
    process_sleep,      // 31
    process_wakeup,     // 32
    process_wait,       // 33
    process_kill,       // 34
    process_get_pid,    // 35
    process_info,   // 36
    process_list_all,   // 37

    // system.h
    pok_load,           // 38
    pok_unload,         // 39

    // scubasuit.h
    scuba_call_generate,// 40
    scuba_call_map,     // 41
    scuba_call_unmap,   // 42
    scuba_call_phys,    // 43
};

void syscall_handler(registers_t *r) {
    uint32_t syscall_id = r->eax;

    if (syscall_id >= (sizeof(SYSCALL_ARRAY) / sizeof(SYSCALL_ARRAY[0]))) {
        sys_error("syscall %d not found\n", syscall_id);
        return;
    }

    uint32_t (*func)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) = SYSCALL_ARRAY[syscall_id];

    if ((void *) func == process_fork) {
        r->eax = process_fork(r);
        return;
    }

    r->eax = func(r->ebx, r->ecx, r->edx, r->esi, r->edi);

    return;
}
