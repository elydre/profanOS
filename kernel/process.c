#include <kernel/process.h>
#include <driver/serial.h>
#include <minilib.h>
#include <system.h>


static process_t plist[PROCESS_MAX];
int pid_order[PROCESS_MAX];

int pid_incrament, need_clean, pid_running;
int pid_order_i = -1;

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
    // this function is called when a process is
    // switched so we don't need security checks

    process_t *proc1 = &plist[i_pid_to_place(pid1)];
    process_t *proc2 = &plist[i_pid_to_place(pid2)];

    if (proc1->state == PROCESS_RUNNING) {
        proc1->state = PROCESS_WAITING;
    }
    proc2->state = PROCESS_RUNNING;

    pid_running = pid2;

    asm volatile("sti"); // (re)enable interrupts
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
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].state == PROCESS_KILLED) {
            free((void *) plist[i].esp_addr);
            plist[i].state = PROCESS_DEAD;
        }
    }
    need_clean = 0;
}

void i_process_yield(int current_pid) {
    if (current_pid == -1) {
        sys_fatal("The current process is set to -1");
        return;
    }

    if (pid_order_i == -1) return;

    int next_pid;

    pid_order_i++;
    if (pid_order[pid_order_i] == -1) pid_order_i = 0;

    next_pid = pid_order[pid_order_i];

    if (next_pid == -1) {
        serial_debug("PROCESS", "Nothing to switch to, adding kernel");
        i_pid_order_add(0);
        next_pid = 0;
    }

    if (current_pid == next_pid) return;

    i_process_switch(current_pid, next_pid);
}

/***********************
 * EXTERNAL FUNCTIONS *
***********************/

int process_init() {
    for (int i = 0; i < PROCESS_MAX; i++) {
        plist[i].state = PROCESS_DEAD;
        pid_order[i] = -1;
    }
    need_clean = 0;

    static process_t kern_proc;

    // Get EFLAGS and CR3
    asm volatile(
        "movl %%cr3, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (kern_proc.regs.cr3)
        :: "%eax");

    asm volatile("pushfl\n\t"
        "movl (%%esp), %%eax\n\t"
        "movl %%eax, %0\n\t"
        "popfl"
        : "=m"(kern_proc.regs.eflags)
        :: "%eax");

    str_cpy(kern_proc.name, "kernel");
    kern_proc.state = PROCESS_RUNNING;

    kern_proc.pid = 0;
    kern_proc.ppid = 0; // it's worse than inbreeding!

    plist[0] = kern_proc;

    i_pid_order_add(0);

    pid_incrament = 0;
    pid_running = 0;
    pid_order_i = 0;

    return 0;
}

int process_create(void (*func)(), char *name) {
    int place = i_get_free_place();

    if (place < 0) {
        sys_error("Too many processes");
        return -1;
    }

    pid_incrament++;

    static process_t new_proc;
    process_t *kern_proc = &plist[0];

    str_cpy(new_proc.name, name);
    new_proc.pid = pid_incrament;
    new_proc.ppid = process_get_running_pid();
    new_proc.state = PROCESS_SLEEPING;

    i_new_process(&new_proc, func, kern_proc->regs.eflags, (uint32_t *) kern_proc->regs.cr3);

    plist[place] = new_proc;

    return pid_incrament;
}

void process_kill(int pid) {
    if (pid == 0) {
        sys_error("Cannot kill kernel (^_^ )");
    }

    int place = i_pid_to_place(pid);

    if (plist[place].state > PROCESS_SLEEPING) {
        sys_error("Process already dead");
    }

    if (place < 0) {
        sys_error("Process not found");
    }

    plist[place].state = PROCESS_KILLED;
    int current_pid = process_get_running_pid();
    i_pid_order_remove(pid);
    need_clean = 1;

    if (pid == current_pid) {
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

    plist[place].state = PROCESS_SLEEPING;
    int current_pid = process_get_running_pid();
    i_pid_order_remove(pid);

    if (pid == current_pid) {
        i_process_yield(pid);
    }
}

void process_wakeup(int pid) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return;
    }

    if (plist[place].state == PROCESS_DEAD) {
        sys_error("Process already dead");
        return;
    }

    if (plist[place].state != PROCESS_SLEEPING) {
        sys_error("Process not sleeping");
        return;
    }

    plist[place].state = PROCESS_WAITING;
    i_pid_order_add(pid);
}

void schedule() {
    if (need_clean) i_clean_killed_process();
    i_process_yield(process_get_running_pid());
}

void process_exit() {
    process_kill(process_get_running_pid());
}

/************************
 * GET / SET FUNCTIONS *
************************/

int process_get_running_pid() {
    return pid_running;
}

void process_set_bin_mem(int pid, uint8_t *mem) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return;
    }

    plist[place].run_mem = mem;
}

uint8_t *process_get_bin_mem(int pid) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return 0;
    }

    return plist[place].run_mem;
}

int process_get_ppid(int pid) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return 0;
    }

    return plist[place].ppid;
}

int process_generate_pid_list(int *list, int max) {
    int i = 0;
    for (int j = 0; j < PROCESS_MAX; j++) {
        if (plist[j].state != PROCESS_DEAD) {
            list[i] = plist[j].pid;
            i++;
        }
        if (max == i) break;
    }
    return i;
}

int process_get_name(int pid, char *name) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return 0;
    }

    str_cpy(name, plist[place].name);
    return 1;
}

int process_get_state(int pid) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return 0;
    }

    return plist[place].state;
}

void *process_get_custom(int pid) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return 0;
    }

    return plist[place].custom;
}

void process_set_custom(int pid, void *custom) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return;
    }

    plist[place].custom = custom;
}
