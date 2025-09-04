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
#include <kernel/afft.h>
#include <drivers/rtc.h>
#include <cpu/timer.h>
#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <minilib.h>
#include <system.h>

#include <minilib.h>
#include <system.h>

void dummy_syscall(void) {;}

void *SYSCALL_ARRAY[] = {
    // butterfly.h
    fs_get_filesys,        // 0
    fs_cnt_read,           // 1
    fs_cnt_write,          // 2
    fs_cnt_set_size,       // 3
    fs_cnt_get_size,       // 4
    fs_cnt_delete,         // 5
    fs_cnt_init,           // 6
    fs_cnt_meta,           // 7

    // snowflake.h
    mem_alloc,             // 8
    mem_free_addr,         // 9
    mem_free_all,          // 10
    mem_alloc_fetch,       // 11
    mem_get_info,          // 12

    // rtc.h + timer.h
    timer_get_ms,          // 13
    time_get,              // 14

    // gnrtx.h + vesa.h
    font_get,              // 15
    kcnprint,              // 16
    cursor_get_offset,     // 17
    vesa_get_info,         // 18

    afft_read,             // 19
    afft_write,            // 20
    afft_cmd,              // 21

    // keyboard.h + mouse.h
    kb_get_scfh,           // 22
    dummy_syscall,         // 23

    // system.h
    sys_power,             // 24
    elf_exec,              // 25

    // process.h + runtime.h
    process_create,        // 26
    process_fork,          // 27
    process_sleep,         // 28
    process_wakeup,        // 29
    process_wait,          // 30
    process_kill,          // 31
    process_get_pid,       // 32
    process_info,          // 33
    process_list_all,      // 34

    // system.h
    mod_load,              // 35
    mod_unload,            // 36

    // scubasuit.h
    scuba_call_generate,   // 37
    scuba_call_map,        // 38
    scuba_call_unmap,      // 39
    scuba_call_phys,       // 40
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
