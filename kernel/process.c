#include <kernel/process.h>
#include <driver/serial.h>
#include <minilib.h>
#include <system.h>

#define ERROR_CODE (-1)

#define SHDLR_ENBL 0    // sheduler enabled
#define SHDLR_TDIS 1    // sheduler temporary disabled
#define SHDLR_ADIS 2    // sheduler absolutely disabled

process_t *plist;

int *sheduler_queue;

int pid_incrament;
int pid_current;

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

void i_process_switch(int pid1, int pid2) {
    // this function is called when a process is
    // switched so we don't need security checks

    process_t *proc1 = &plist[i_pid_to_place(pid1)];
    process_t *proc2 = &plist[i_pid_to_place(pid2)];

    if (proc1->state == PROCESS_RUNNING) {
        proc1->state = PROCESS_WAITING;
    }
    proc2->state = PROCESS_RUNNING;

    pid_current = pid2;

    asm volatile("sti"); // (re)enable interrupts
    process_asm_switch(&proc1->regs, &proc2->regs);
}

void i_add_to_sheduler_queue(int pid, int priority) {
    
}

/*********************
 * PUBLIC FUNCTIONS *
*********************/

int process_init() {
    plist = calloc(sizeof(process_t) * PROCESS_MAX);
    sheduler_queue = calloc(sizeof(int) * PROCESS_MAX * 10);

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

    sheduler_state = SHDLR_ENBL;

    return 0;
}

int process_get_pid() {
    return pid_current;
}

int process_create(void (*func)(), int priority, char *name) {
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

void schedule() {
    if (sheduler_state) {
        serial_debug("SHEDULER", "currently disabled, can't schedule");
        return;
    }


    
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
