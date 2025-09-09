/*****************************************************************************\
|   === process.h : 2024 ===                                                  |
|                                                                             |
|    Kernel process manager v2.1 header                            .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef PROCESS_H
#define PROCESS_H

#include <kernel/scubasuit.h>
#include <ktype.h>

#define PROC_STATE_FRE 0
#define PROC_STATE_ZMB 1
#define PROC_STATE_SLP 2
#define PROC_STATE_INQ 3
#define PROC_STATE_RUN 4
#define PROC_STATE_IDL 5

#define PROC_INFO_PPID     0
#define PROC_INFO_STATE    1
#define PROC_INFO_SLEEP_TO 2
#define PROC_INFO_RUN_TIME 3
#define PROC_INFO_NAME     4
#define PROC_INFO_STACK    5
#define PROC_INFO_SET_NAME 6

#define process_get_ppid(pid)  process_info(pid, PROC_INFO_PPID, NULL)
#define process_get_state(pid) process_info(pid, PROC_INFO_STATE, NULL)

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} proc_rgs_t;

typedef struct {
    proc_rgs_t regs;
    scuba_dir_t *scuba_dir;

    uint32_t pid, ppid, run_time;

    int wait_pid;

    union {
        uint32_t sleep_to;
        int retcode;
    };

    uint8_t state, in_kernel;

    char name[64];
} process_t;

// setup
int process_init(void);

// scheduler
void schedule_if_needed(void);
void schedule(uint32_t ticks);

// process gestion
int process_create(void *func, int copy_page, int nargs, uint32_t *args);
int process_fork(registers_t *regs);
int process_wakeup(uint32_t pid, int handover);
int process_sleep(uint32_t pid, uint32_t ms);
int process_wait(int pid, uint8_t *retcode, int block); // negative pid for any child
int process_kill(uint32_t pid, uint8_t retcode);

// process info
uint32_t process_get_pid(void);

int process_list_all(uint32_t *list, int max);
int process_info(uint32_t pid, int info_id, void *ptr);

scuba_dir_t *process_get_dir(uint32_t pid);
void process_switch_directory(uint32_t pid, scuba_dir_t *dir, int now);

// switch.asm
extern void process_asm_switch(proc_rgs_t *old, proc_rgs_t *new);

int process_is_dead(int pid);

#endif
