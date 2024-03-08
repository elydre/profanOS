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
#define PROCESS_INFO_PRIORITY   1
#define PROCESS_INFO_STATE      2
#define PROCESS_INFO_SLEEP_TO   3
#define PROCESS_INFO_RUN_TIME   4
#define PROCESS_INFO_EXIT_CODE  5
#define PROCESS_INFO_NAME       6
#define PROCESS_INFO_STACK      7

#define process_disable_scheduler() process_set_scheduler(0)
#define process_enable_scheduler()  process_set_scheduler(1)

#define process_get_ppid(pid) process_get_info(pid, PROCESS_INFO_PPID)
#define process_get_state(pid) process_get_info(pid, PROCESS_INFO_STATE)

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} proc_rgs_t;

typedef struct {
    proc_rgs_t regs;
    scuba_directory_t *scuba_dir;

    uint32_t pid, ppid, use_parent_dir;
    uint32_t priority, run_time;
    uint32_t esp_addr, sleep_to, state;

    char name[64];
    comm_struct_t *comm;
} process_t;

// setup and call
int  process_init(void);
void schedule(uint32_t ticks);

// process gestion
int process_create(void *func, int use_parent_dir, char *name, int nargs, ...);
int process_fork(void);
int process_handover(uint32_t pid);
int process_wakeup(uint32_t pid);
int process_sleep(uint32_t pid, uint32_t ms);
int process_kill(uint32_t pid);

// scheduler control
void process_set_scheduler(int state);

// process info
uint32_t process_get_pid(void);

int process_generate_pid_list(uint32_t *list, int max);
uint32_t process_get_info(uint32_t pid, int info_id);

int process_set_comm(uint32_t pid, comm_struct_t *comm);
comm_struct_t *process_get_comm(uint32_t pid);

int process_set_priority(uint32_t pid, int priority);
int process_set_return(uint32_t pid, uint32_t ret);

scuba_directory_t *process_get_directory(uint32_t pid);
void process_switch_directory(uint32_t pid, scuba_directory_t *dir);

// switch.asm
extern void process_asm_switch(proc_rgs_t *old, proc_rgs_t *new);

#endif
