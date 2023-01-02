#include <kernel/process.h>
#include <driver/serial.h>
#include <minilib.h>
#include <system.h>


static process_t plist[PROCESS_MAX];
int pid_order[PROCESS_MAX];
int pid_order_i = -1;
int current_pid, need_clean;

#define PROCESS_RUNNING  0
#define PROCESS_WAITING  1
#define PROCESS_SLEEPING 2
#define PROCESS_KILLED   3
#define PROCESS_DEAD     4

/***********************
 * INTERNAL FUNCTIONS *
***********************/

void i_new_process(process_t *process, void (*func)(), uint32_t flags, uint32_t *pagedir) {
    uint32_t esp_alloc = (uint32_t) malloc(PROCESS_ESP);
    process->regs.eax = 0;
    process->regs.ebx = 0;
    process->regs.ecx = 0;
    process->regs.edx = 0;
    process->regs.esi = 0;
    process->regs.edi = 0;
    process->regs.eflags = flags;
    process->regs.eip = (uint32_t) func;
    process->regs.cr3 = (uint32_t) pagedir;
    process->regs.esp = esp_alloc + PROCESS_ESP;
    process->esp_addr = esp_alloc;
}

int i_get_free_place() {
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].state == PROCESS_DEAD) return i;
    }
    return -1;
}

int i_pid_to_place(int pid) {
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].pid == pid) return i;
    }
    return -1;
}

void i_process_switch(int pid1, int pid2) {
    // this function is called when a process 
    // is switched so we need security checks

    process_t *proc1 = &plist[i_pid_to_place(pid1)];
    process_t *proc2 = &plist[i_pid_to_place(pid2)];

    if (proc1->state == PROCESS_RUNNING) {
        proc1->state = PROCESS_WAITING;
    }
    proc2->state = PROCESS_RUNNING;

    process_asm_switch(&proc1->regs, &proc2->regs);
}

void i_pid_order_add(int pid) {
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (pid_order[i] == -1) {
            pid_order[i] = pid;
            return;
        }
    }
}

void i_pid_order_remove(int pid) {
    int i;
    for (i = 0; i < PROCESS_MAX; i++) {
        if (pid_order[i] == pid) {
            break;
        }
    }
    for (; i < PROCESS_MAX - 1; i++) {
        pid_order[i] = pid_order[i + 1];
    }
    pid_order[PROCESS_MAX - 1] = -1;
}

void i_clean_killed_process() {
    sprintf("Cleaning killed processes...\n");
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].state == PROCESS_KILLED) {
            free((void *) plist[i].esp_addr);
            plist[i].state = PROCESS_DEAD;
            sprintf("Process %d cleaned\n", plist[i].pid);
        }
    }
    need_clean = 0;
}

void i_process_yield(int current_pid) {
    if (pid_order_i == -1) return;

    int next_pid;

    pid_order_i++;
    if (pid_order[pid_order_i] == -1) pid_order_i = 0;

    next_pid = pid_order[pid_order_i];

    if (current_pid == next_pid) return;

    sprintf("Switching from %d to %d\n", current_pid, next_pid);

    i_process_switch(current_pid, next_pid);
}

/***********************
 * EXTERNAL FUNCTIONS *
***********************/

int process_get_current_pid() {
    return pid_order[pid_order_i];
}

int process_init() {
    for (int i = 0; i < PROCESS_MAX; i++) {
        plist[i].state = PROCESS_DEAD;
        pid_order[i] = -1;
    }
    need_clean = 0;

    static process_t main_proc;

    // Get EFLAGS and CR3
    asm volatile(
        "movl %%cr3, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (main_proc.regs.cr3)
        :: "%eax");

    asm volatile("pushfl\n\t"
        "movl (%%esp), %%eax\n\t"
        "movl %%eax, %0\n\t"
        "popfl"
        : "=m"(main_proc.regs.eflags)
        :: "%eax");

    str_cpy(main_proc.name, "kernel");
    main_proc.state = PROCESS_RUNNING;
    main_proc.pid = 0;

    plist[0] = main_proc;

    i_pid_order_add(0);

    current_pid = 0;
    pid_order_i = 0;

    return 0;
}

int process_create(void (*func)(), char *name) {
    int place = i_get_free_place();

    if (place < 0) {
        sys_error("Too many processes");
        return -1;
    }

    current_pid++;

    static process_t new_proc;
    process_t *main_proc = &plist[0];

    str_cpy(new_proc.name, name);
    new_proc.pid = current_pid;
    new_proc.state = PROCESS_WAITING;

    i_new_process(&new_proc, func, main_proc->regs.eflags, (uint32_t *) main_proc->regs.cr3);

    plist[place] = new_proc;

    i_pid_order_add(current_pid);

    return current_pid;
}

void process_kill(int pid) {
    sprintf("Killing process %d\n", pid);
    process_debug();

    if (pid == 0) {
        sys_error("Cannot kill kernel");
    }

    int place = i_pid_to_place(pid);

    if (plist[place].state > PROCESS_SLEEPING) {
        sys_error("Process already dead");
    }

    if (place < 0) {
        sys_error("Process not found");
    }

    plist[place].state = PROCESS_KILLED;

    if (pid == process_get_current_pid()) {
        sprintf("Killing current process %d\n", pid);
        i_pid_order_remove(pid);
        need_clean = 1;
        i_process_yield(pid);
    }
}

void process_sleep(int pid) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return;
    }

    if (plist[place].state == PROCESS_DEAD) {
        sys_error("Process already dead");
        return;
    }

    if (plist[place].state == PROCESS_SLEEPING) {
        sys_error("Process already sleeping");
        return;
    }

    plist[place].state = PROCESS_WAITING;
    i_pid_order_remove(pid);

    if (pid == process_get_current_pid()) {
        schedule();
    }
}

void schedule() {
    if (need_clean) i_clean_killed_process();
    i_process_yield(process_get_current_pid());
}

void process_exit() {
    process_kill(process_get_current_pid());
}

void process_debug() {
    sprintf("Current index: %d [", pid_order_i);
    for (int i = 0; i < PROCESS_MAX; i++) {
        sprintf("%d ", pid_order[i]);
    }
    sprintf("]\n");

    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].state != PROCESS_DEAD) {
            sprintf("Process %d: %s, state: %d\n", plist[i].pid, plist[i].name, plist[i].state);
        }
    }
}
