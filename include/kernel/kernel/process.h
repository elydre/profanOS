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

#define process_disable_sheduler() process_set_sheduler(0)
#define process_enable_sheduler()  process_set_sheduler(1)


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
    void *comm;
} process_t;

// setup and call
int  process_init(void);
void schedule(uint32_t ticks);

// process gestion
int process_create(void (*func)(void), int use_parent_dir, char *name);
int process_handover(uint32_t pid);
int process_wakeup(uint32_t pid);
int process_sleep(uint32_t pid, uint32_t ms);
int process_kill(uint32_t pid);

// sheduler control
void process_set_sheduler(int state);

// process info
int process_get_pid(void);
int process_get_ppid(uint32_t pid);
int process_get_state(uint32_t pid);
int process_generate_pid_list(uint32_t *list, int max);
int process_get_name(uint32_t pid, char *name);
uint32_t process_get_run_time(uint32_t pid);

void  process_set_comm(uint32_t pid, void *comm);
void *process_get_comm(uint32_t pid);

void process_set_priority(uint32_t pid, int priority);
int  process_get_priority(uint32_t pid);

scuba_directory_t *process_get_directory(uint32_t pid);

// switch.asm
extern void process_asm_switch(proc_rgs_t *old, proc_rgs_t *new);

#endif
