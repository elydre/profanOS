#include <kernel/process.h>
#include <driver/serial.h>
#include <minilib.h>
#include <system.h>

#define ERROR_CODE (-1)

#define SHDLR_ENBL 0    // sheduler enabled
#define SHDLR_TDIS 1    // sheduler temporary disabled
#define SHDLR_ADIS 2    // sheduler absolutely disabled

process_t *plist;

int pid_incrament;
int pid_current;

int *shdlr_queue;
int shdlr_queue_index;
int shdlr_queue_length;

uint8_t sheduler_state = SHDLR_ADIS;



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
    return ERROR_CODE;
}

int i_pid_to_place(int pid) {
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].pid == pid) return i;
    }
    return ERROR_CODE;
}

void i_process_switch(int from_pid, int to_pid) {
    // this function is called when a process is
    // switched so we don't need security checks

    process_t *proc1 = &plist[i_pid_to_place(from_pid)];
    process_t *proc2 = &plist[i_pid_to_place(to_pid)];

    if (proc1->state == PROCESS_RUNNING) {
        proc1->state = PROCESS_WAITING;
    }
    proc2->state = PROCESS_RUNNING;

    pid_current = to_pid;

    sprintf("Switching from %d to %d\n", from_pid, to_pid);

    asm volatile("sti"); // (re)enable interrupts
    process_asm_switch(&proc1->regs, &proc2->regs);
}

int i_add_to_shdlr_queue(int pid, int priority) {
    if (shdlr_queue_length + priority > PROCESS_MAX * 10) {
        return ERROR_CODE;
    }

    for (int i = 0; i < priority; i++) {
        shdlr_queue[shdlr_queue_length + i] = pid;
    }

    shdlr_queue_length += priority;

    return 0;
}

/*********************
 * PUBLIC FUNCTIONS *
*********************/

int process_init() {
    plist = calloc(sizeof(process_t) * PROCESS_MAX);
    shdlr_queue = calloc(sizeof(int) * PROCESS_MAX * 10);

    for (int i = 0; i < PROCESS_MAX; i++) {
        plist[i].state = PROCESS_DEAD;
    }

    process_t *kern_proc = &plist[0];

    // Get EFLAGS and CR3
    asm volatile (
        "movl %%cr3, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (kern_proc->regs.cr3)
        :: "%eax"
    );

    asm volatile (
        "pushfl\n\t"
        "movl (%%esp), %%eax\n\t"
        "movl %%eax, %0\n\t"
        "popfl"
        : "=m"(kern_proc->regs.eflags)
        :: "%eax"
    );

    str_cpy(kern_proc->name, "kernel");
    kern_proc->state = PROCESS_RUNNING;

    kern_proc->pid = 0;
    kern_proc->ppid = 0; // it's worse than inbreeding!

    pid_incrament = 0;
    pid_current = 0;

    shdlr_queue_index = 0;
    shdlr_queue_length = 0;

    i_add_to_shdlr_queue(0, 5);

    sheduler_state = SHDLR_ENBL;

    return 0;
}

int process_get_pid() {
    return pid_current;
}

int process_create(void (*func)(), int priority, char *name) {
    if (priority > 10) {
        sys_error("Priority can't be higher than 10");
        return ERROR_CODE;
    }

    int place = i_get_free_place();

    if (place == ERROR_CODE) {
        sys_error("Too many processes");
        return ERROR_CODE;
    }

    pid_incrament++;

    process_t *kern_proc = &plist[0];
    process_t *new_proc = &plist[place];

    str_cpy(new_proc->name, name);
    new_proc->pid = pid_incrament;
    new_proc->ppid = process_get_pid();
    new_proc->state = PROCESS_SLEEPING;
    new_proc->priority = priority;

    i_new_process(new_proc, func, kern_proc->regs.eflags, (uint32_t *) kern_proc->regs.cr3);

    return pid_incrament;
}

int process_wakeup(int pid) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_DEAD) {
        sys_error("Process already dead");
        return ERROR_CODE;
    }

    if (plist[place].state != PROCESS_SLEEPING) {
        sys_error("Process not sleeping");
        return ERROR_CODE;
    }

    plist[place].state = PROCESS_WAITING;
    if (i_add_to_shdlr_queue(pid, plist[place].priority) == ERROR_CODE) {
        sys_error("Can't add process to sheduler queue");
        return ERROR_CODE;
    }
    
    return 0;
}

void schedule() {
    if (sheduler_state) {
        serial_debug("SHEDULER", "currently disabled, can't schedule");
        return;
    }


    shdlr_queue_index++;
    if (shdlr_queue_index >= shdlr_queue_length) {
        shdlr_queue_index = 0;
    }
    int pid = shdlr_queue[shdlr_queue_index];

    if (pid == pid_current) {
        serial_debug("SHEDULER", "process is already running");
        return;
    }

    i_process_switch(pid_current, pid);
    
    
    if (sheduler_state == SHDLR_TDIS) {
        sheduler_state = SHDLR_ENBL;
    } else if (sheduler_state == SHDLR_ADIS) {   // DEBUG
        sys_error("this should never happen (schedule)");
    }
}

void process_enable_sheduler() {
    sheduler_state = SHDLR_ENBL;
}

void process_disable_sheduler() {
    sheduler_state = SHDLR_ADIS;
}
