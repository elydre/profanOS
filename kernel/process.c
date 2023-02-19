#include <kernel/snowflake.h>
#include <kernel/process.h>
#include <cpu/timer.h>
#include <minilib.h>
#include <system.h>

#define ERROR_CODE (-1)

#define SHDLR_ENBL 0    // sheduler enabled
#define SHDLR_RUNN 1    // sheduler running
#define SHDLR_DISL 2    // sheduler disabled

#define SCHEDULER_EVRY (RATE_TIMER_TICK / RATE_SCHEDULER)

process_t *plist;

process_t **tsleep_list;
uint32_t tsleep_interact;
int tsleep_list_length;

int shdlr_queue_length;
int *shdlr_queue;

uint8_t sheduler_state = SHDLR_DISL;
uint8_t need_clean;
int pid_current;


/***********************
 * INTERNAL FUNCTIONS *
***********************/

void i_new_process(process_t *process, void (*func)(), uint32_t flags, uint32_t *pagedir) {
    uint32_t esp_alloc = (uint32_t) mem_alloc(PROCESS_ESP, 6);
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

void i_process_switch(int from_pid, int to_pid, uint32_t ticks) {
    // this function is called when a process is
    // switched so we don't need security checks

    process_t *proc1 = &plist[i_pid_to_place(from_pid)];
    process_t *proc2 = &plist[i_pid_to_place(to_pid)];

    if (!ticks) {
        ticks = timer_get_ticks();
    }

    static uint32_t last_switch = 0;

    proc1->run_time += ticks - last_switch;
    last_switch = ticks;

    if (proc1->state == PROCESS_RUNNING) {
        proc1->state = PROCESS_WAITING;
    }
    if (proc2->state != PROCESS_IDLETIME) {
        proc2->state = PROCESS_RUNNING;
    }

    pid_current = to_pid;

    asm volatile("sti"); // (re)enable interrupts
    process_asm_switch(&proc1->regs, &proc2->regs);
}

void i_optimize_shdlr_queue() {
    // TODO: separate all occurrences of each number as much as possible
    return;
}

int i_add_to_shdlr_queue(int pid, int priority) {
    if (shdlr_queue_length + priority > PROCESS_MAX * 10) {
        sys_error("process queue is full");
        return ERROR_CODE;
    }

    for (int i = 0; i < priority; i++) {
        shdlr_queue[shdlr_queue_length + i] = pid;
    }

    shdlr_queue_length += priority;

    i_optimize_shdlr_queue();

    return 0;
}

int i_remove_from_shdlr_queue(int pid) {
    // remove all occurences of pid from shdlr_queue
    for (int i = 0; i < shdlr_queue_length; i++) {
        if (shdlr_queue[i] == pid) {
            shdlr_queue[i] = shdlr_queue[shdlr_queue_length - 1];
            shdlr_queue_length--;
            i--;
        }
    }

    i_optimize_shdlr_queue();

    return 0;
}

void i_clean_killed() {
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].state == PROCESS_KILLED) {
            free((void *) plist[i].esp_addr);
            plist[i].state = PROCESS_DEAD;
        }
    }
    need_clean = 0;
}

void i_refresh_tsleep_interact() {
    tsleep_interact = 0;
    for (int i = 0; i < tsleep_list_length; i++) {
        if (tsleep_list[i]->sleep_to < tsleep_interact || !tsleep_interact) {
            tsleep_interact = tsleep_list[i]->sleep_to;
        }
    }
}

void i_exit_sheduler() {
    if (sheduler_state == SHDLR_RUNN) {
        sheduler_state = SHDLR_ENBL;
    } else {
        sys_error("sheduler is not running but sheduler is exiting");
    }
}

void i_tsleep_awake(uint32_t ticks) {
    for (int i = 0; i < tsleep_list_length; i++) {
        if (tsleep_list[i]->sleep_to <= ticks) {
            if (tsleep_list[i]->state == PROCESS_TSLPING) {
                tsleep_list[i]->state = PROCESS_WAITING;
                i_add_to_shdlr_queue(tsleep_list[i]->pid, tsleep_list[i]->priority);
                tsleep_list[i] = tsleep_list[tsleep_list_length - 1];
                tsleep_list_length--;
                i--;
            } else {
                sys_error("process in tsleep list is not in tsleep state");
            }
        }
    }
    i_refresh_tsleep_interact();
}

void i_remove_from_tsleep_list(int pid) {
    for (int i = 0; i < tsleep_list_length; i++) {
        if (tsleep_list[i]->pid == pid) {
            tsleep_list[i] = tsleep_list[tsleep_list_length - 1];
            tsleep_list_length--;
            i--;
        }
    }
    i_refresh_tsleep_interact();
}

/*****************
 * IDLE PROCESS *
*****************/

void idle_process() {
    while (1) {
        asm volatile("hlt");
    }
}

/*********************
 * PUBLIC FUNCTIONS *
*********************/

int process_init() {
    plist = calloc(sizeof(process_t) * PROCESS_MAX);
    shdlr_queue = calloc(sizeof(int) * PROCESS_MAX * 10);
    tsleep_list = calloc(sizeof(process_t *) * PROCESS_MAX);

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
    kern_proc->priority = KERNEL_PRIORITY;

    tsleep_interact = 0;
    pid_current = 0;
    need_clean = 0;

    shdlr_queue_length = 0;

    i_add_to_shdlr_queue(0, KERNEL_PRIORITY);

    // enable sheduler
    sheduler_state = SHDLR_ENBL;

    // create idle process
    process_create(idle_process, 1, "idle");
    plist[1].state = PROCESS_IDLETIME;

    return 0;
}


int process_create(void (*func)(), int priority, char *name) {
    if (priority > 10) {
        sys_error("Priority can't be higher than 10");
        return ERROR_CODE;
    }

    if (priority < 1) {
        sys_error("Priority can't be lower than 1");
        return ERROR_CODE;
    }

    int place = i_get_free_place();

    if (place == ERROR_CODE) {
        sys_error("Too many processes");
        return ERROR_CODE;
    }

    static int pid_incrament = 0;
    pid_incrament++;

    process_t *kern_proc = &plist[0];
    process_t *new_proc = &plist[place];

    str_cpy(new_proc->name, name);
    new_proc->pid = pid_incrament;
    new_proc->ppid = pid_current;
    new_proc->state = PROCESS_FSLPING;
    new_proc->priority = priority;

    i_new_process(new_proc, func, kern_proc->regs.eflags, (uint32_t *) kern_proc->regs.cr3);

    return pid_incrament;
}


int process_sleep(int pid, uint32_t ms) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_IDLETIME) {
        sys_error("Can't interact with idle process");
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_FSLPING) {
        sys_error("Process already sleeping");
        return ERROR_CODE;
    }

    if (plist[place].state >= PROCESS_KILLED) {
        sys_error("Process already dead");
        return ERROR_CODE;
    }

    process_disable_sheduler();

    if (plist[place].state == PROCESS_TSLPING) {
        i_remove_from_tsleep_list(pid);
    }

    if (ms == 0) {
        plist[place].state = PROCESS_FSLPING;
    } else {
        plist[place].state = PROCESS_TSLPING;
        // convert ms to ticks
        plist[place].sleep_to = timer_get_ticks() + (ms * 1000 / RATE_TIMER_TICK);
        tsleep_list[tsleep_list_length] = &plist[place];
        tsleep_list_length++;
        i_refresh_tsleep_interact();
    }

    i_remove_from_shdlr_queue(pid);

    process_enable_sheduler();
    if (pid == pid_current) {
        schedule(0);
    }

    return 0;
}


int process_wakeup(int pid) {   // TODO: sleep to exit gestion
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_IDLETIME) {
        sys_error("Can't interact with idle process");
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_DEAD) {
        sys_error("Process already dead");
        return ERROR_CODE;
    }

    if (!(plist[place].state == PROCESS_FSLPING || plist[place].state == PROCESS_TSLPING)) {
        sys_error("Process not sleeping");
        return ERROR_CODE;
    }

    process_disable_sheduler();

    if (plist[place].state == PROCESS_TSLPING) {
        i_remove_from_tsleep_list(pid);
    }
    
    plist[place].state = PROCESS_WAITING;
    i_add_to_shdlr_queue(pid, plist[place].priority);

    i_refresh_tsleep_interact();

    process_enable_sheduler();
    
    return 0;
}


int process_handover(int pid) {
    int place = i_pid_to_place(pid);
    int current_place = i_pid_to_place(pid_current);

    if (place < 0) {
        sys_error("Process not found");
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_IDLETIME) {
        sys_error("Can't interact with idle process");
        return ERROR_CODE;
    }

    if (plist[current_place].state == PROCESS_IDLETIME) {
        process_wakeup(pid);
        return 0;
    }

    if (plist[place].state == PROCESS_DEAD) {
        sys_error("Process already dead");
        return ERROR_CODE;
    }

    if (!(plist[place].state == PROCESS_FSLPING || plist[place].state == PROCESS_TSLPING)) {
        sys_error("Process not sleeping");
        return ERROR_CODE;
    }

    process_disable_sheduler();

    if (plist[place].state == PROCESS_TSLPING) {
        i_remove_from_tsleep_list(pid);
    }

    plist[place].state = PROCESS_WAITING;
    i_add_to_shdlr_queue(pid, plist[place].priority);

    if (plist[current_place].state == PROCESS_TSLPING) {
        i_remove_from_tsleep_list(pid_current);
    }

    plist[current_place].state = PROCESS_FSLPING;
    i_remove_from_shdlr_queue(pid_current);

    process_enable_sheduler();

    schedule(0);
    
    return 0;
}


int process_kill(int pid) {
    if (pid == 0) {
        sys_error("Cannot kill kernel (^_^ )");
        return ERROR_CODE;
    }

    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_IDLETIME) {
        sys_error("Can't interact with idle process");
        return ERROR_CODE;
    }

    if (plist[place].state >= PROCESS_KILLED) {
        sys_error("Process already dead");
        return ERROR_CODE;
    }

    process_disable_sheduler();

    if (plist[place].state == PROCESS_TSLPING) {
        i_remove_from_tsleep_list(pid);
    }

    plist[place].state = PROCESS_KILLED;
    i_remove_from_shdlr_queue(pid);

    need_clean = 1;

    process_enable_sheduler();

    if (pid == pid_current) {
        schedule(0);
    }

    return 0;
}


int process_exit() {
    return process_kill(pid_current);
}


/************************
 * SCHEUDLER FUNCTIONS *
************************/

void process_enable_sheduler() {
    if (sheduler_state == SHDLR_DISL) {
        sheduler_state = SHDLR_ENBL;
    }
}

void process_disable_sheduler() {
    if (sheduler_state == SHDLR_ENBL) {
        sheduler_state = SHDLR_DISL;
    }
}

void schedule(uint32_t ticks) {
    if (sheduler_state != SHDLR_ENBL) {
        return;
    }

    sheduler_state = SHDLR_RUNN;

    if (tsleep_interact && tsleep_interact <= ticks) {
        i_tsleep_awake(ticks);
    }

    if (ticks % SCHEDULER_EVRY) {
        i_exit_sheduler();
        return;
    }

    if (need_clean) {
        i_clean_killed();
    }

    static int shdlr_queue_index = 0;
    shdlr_queue_index++;

    if (shdlr_queue_index >= shdlr_queue_length) {
        shdlr_queue_index = 0;
    }

    int pid;

    if (shdlr_queue_length == 0) {
        pid = 1;    // idle process
    } else {
        pid = shdlr_queue[shdlr_queue_index];
    }

    i_exit_sheduler();

    if (pid != pid_current) {
        i_process_switch(pid_current, pid, ticks);
    }
}


/************************
 * GET / SET FUNCTIONS *
************************/

void process_set_priority(int pid, int priority) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return;
    }

    if (priority > 10) {
        sys_error("Priority can't be higher than 10");
        return;
    }

    if (priority < 1) {
        sys_error("Priority can't be lower than 1");
        return;
    }

    plist[place].priority = priority;

    if (plist[place].state < PROCESS_TSLPING) {
        process_disable_sheduler();
        
        i_remove_from_shdlr_queue(pid);
        i_add_to_shdlr_queue(pid, priority);
        
        process_enable_sheduler();
    }
}

int process_get_pid() {
    return pid_current;
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

int process_get_priority(int pid) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return 0;
    }

    return plist[place].priority;
}

uint32_t process_get_run_time(int pid) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_error("Process not found");
        return 0;
    }

    return plist[place].run_time;
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
