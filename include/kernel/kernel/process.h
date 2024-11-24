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

#ifndef TASK_H
#define TASK_H

#include <kernel/scubasuit.h>
#include <ktype.h>

#define PROCESS_RUNNING  0
#define PROCESS_WAITING  1  // in the queue
#define PROCESS_TSLPING  2  // time sleep
#define PROCESS_FSLPING  3  // forever sleep
#define PROCESS_KILLED   4
#define PROCESS_DEAD     5
#define PROCESS_IDLETIME 6

#define PROCESS_INFO_PPID       0
#define PROCESS_INFO_STATE      1
#define PROCESS_INFO_SLEEP_TO   2
#define PROCESS_INFO_RUN_TIME   3
#define PROCESS_INFO_EXIT_CODE  4
#define PROCESS_INFO_NAME       5
#define PROCESS_INFO_STACK      6

#define process_get_ppid(pid) process_get_info(pid, PROCESS_INFO_PPID)
#define process_get_state(pid) process_get_info(pid, PROCESS_INFO_STATE)

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} proc_rgs_t;

typedef struct {
    proc_rgs_t regs;
    scuba_dir_t *scuba_dir;

    uint32_t pid, ppid, state;
    uint32_t run_time, sleep_to;
    int wait_pid;

    uint8_t in_kernel;

    char name[64];
    comm_struct_t *comm;
} process_t;

// setup and call
int  process_init(void);
void schedule(uint32_t ticks);

// process gestion
int process_create(void *func, int copy_page, int nargs, uint32_t *args);
int process_fork(registers_t *regs);
int process_wakeup(uint32_t pid, int handover);
int process_sleep(uint32_t pid, uint32_t ms);
int process_wait(int pid); // negative pid for any child
int process_kill(uint32_t pid);

// scheduler control
int process_auto_schedule(int state);

// process info
uint32_t process_get_pid(void);

int process_list_all(uint32_t *list, int max);
int process_get_info(uint32_t pid, int info_id);

int process_set_comm(uint32_t pid, comm_struct_t *comm);
comm_struct_t *process_get_comm(uint32_t pid);

int process_set_return(uint32_t pid, uint32_t ret);
int process_set_name(uint32_t pid, char *name);

scuba_dir_t *process_get_dir(uint32_t pid);
void process_switch_directory(uint32_t pid, scuba_dir_t *dir, int now);

// switch.asm
extern void process_asm_switch(proc_rgs_t *old, proc_rgs_t *new);

#endif
