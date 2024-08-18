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

#include <kernel/syscall.h>
#include <minilib.h>
#include <system.h>

uint32_t saCduSYScallBAHnonSAfaitJUSTElaSOMME(uint32_t a, uint32_t b) {
    return a + b;
}

void *SYSCALL_ARRAY[] = {
    saCduSYScallBAHnonSAfaitJUSTElaSOMME
};


#define SYSCALL_COUNT 1

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
