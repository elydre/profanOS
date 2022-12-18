#ifndef TASK_H
#define TASK_H

#include <type.h>

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} task_rgs_t;

typedef struct {
    task_rgs_t regs;
    int pid, isdead;
    uint32_t esp_addr;
    uint8_t *bin_mem;
    char name[32];
} task_t;

void tasking_init();
int task_create(void (*func)(), char *name);

void task_switch(int target_pid);
void task_kill_task_switch(int target_pid);
void task_kill(int target_pid);

int task_get_current_pid();
int task_get_next_pid();

int task_get_alive();
int task_get_max();

void task_set_bin_mem(int pid, uint8_t *bin_mem);
uint8_t *task_get_bin_mem(int pid);

char *task_get_name(int internal_pos);
int task_get_pid(int internal_pos);

// switch.asm
extern void task_asm_switch(task_rgs_t *old, task_rgs_t *new);

#endif
