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
#include <net.h>
#include <drivers/rtc.h>
#include <kernel/afft.h>
#include <cpu/timer.h>
#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <minilib.h>
#include <system.h>

#include <minilib.h>
#include <system.h>

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
    vesa_get_info,         // 17

    // afft.h
    afft_read,             // 18
    afft_write,            // 19
    afft_cmd,              // 20

    // keyboard.h
    kb_get_scancode,           // 21

    // system.h
    sys_power,             // 22
    elf_exec,              // 23

    // process.h + runtime.h
    process_create,        // 24
    process_fork,          // 25
    process_sleep,         // 26
    process_wakeup,        // 27
    process_wait,          // 28
    process_kill,          // 29
    process_get_pid,       // 30
    process_info,          // 31
    process_list_all,      // 32

    // system.h
    mod_load,              // 33
    mod_unload,            // 34

    // scubasuit.h
    scuba_call_generate,   // 35
    scuba_call_map,        // 36
    scuba_call_unmap,      // 37
    scuba_call_phys,       // 38

    eth_start,             // 39
    eth_end,               // 40
    eth_send,              // 41
    eth_is_ready,          // 42
    eth_recv,              // 43
    eth_get_info,          // 44
    eth_set_info,          // 45
    eth_get_transaction,   // 46
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
