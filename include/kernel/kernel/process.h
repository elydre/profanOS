#ifndef TASK_H
#define TASK_H

#include <type.h>

#define PROCESS_RUNNING  0
#define PROCESS_WAITING  1  // in the queue
#define PROCESS_TSLPING  2  // time sleep
#define PROCESS_FSLPING  3  // forever sleep
#define PROCESS_KILLED   4
#define PROCESS_DEAD     5


typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} proc_rgs_t;

typedef struct {
    proc_rgs_t regs;
    int pid, ppid, priority, state;
    uint32_t esp_addr, sleep_to, run_time;
    uint8_t *run_mem;
    void *custom;
    char name[64];
} process_t;

// setup and call
int process_init();
void schedule(uint32_t ticks);

// process gestion
int process_create(void (*func)(), int priority, char *name);
int process_handover(int pid);
int process_wakeup(int pid);
int process_sleep(int pid, uint32_t ms);
int process_kill(int pid);
int process_exit();


// sheduler control
void process_enable_sheduler();
void process_disable_sheduler();


// process info
int process_get_pid();

int process_get_ppid(int pid);
int process_get_state(int pid);
int process_generate_pid_list(int *list, int max);
int process_get_name(int pid, char *name);
uint32_t process_get_run_time(int pid);

void process_set_bin_mem(int pid, uint8_t *mem);
uint8_t *process_get_bin_mem(int pid);

void *process_get_custom(int pid);
void process_set_custom(int pid, void *custom);

void process_set_priority(int pid, int priority);
int process_get_priority(int pid);


// switch.asm
extern void process_asm_switch(proc_rgs_t *old, proc_rgs_t *new);

#endif
