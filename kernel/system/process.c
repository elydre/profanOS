#include <kernel/snowflake.h>
#include <kernel/process.h>
#include <cpu/timer.h>
#include <minilib.h>
#include <system.h>

#define ERROR_CODE (-1)

#define SHDLR_ENBL 0    // sheduler enabled
#define SHDLR_RUNN 1    // sheduler running
#define SHDLR_DISL 2    // sheduler disabled
#define SHDLR_DEAD 3    // sheduler dead

#define SCHEDULER_EVRY (RATE_TIMER_TICK / RATE_SCHEDULER)

process_t *plist;

process_t **tsleep_list;
uint32_t tsleep_interact;
int tsleep_list_length;

int shdlr_queue_length;
int *shdlr_queue;

uint8_t sheduler_state = SHDLR_DEAD;
uint8_t sheduler_count;
uint8_t need_clean;
int sheduler_disable_count;

uint32_t pid_current;
uint8_t *exit_codes;

/***********************
 * INTERNAL FUNCTIONS *
***********************/

void i_new_process(process_t *process, void (*func)(), uint32_t flags, uint32_t *pagedir) {
    uint32_t esp_alloc = mem_alloc(PROCESS_ESP, 0, 4);
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

int i_get_free_place(void) {
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].state == PROCESS_DEAD) return i;
    }
    return ERROR_CODE;
}

int i_pid_to_place(uint32_t pid) {
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].pid == pid) return i;
    }
    return ERROR_CODE;
}

void i_end_sheduler(void) {
    if (pid_current != 1) {
        scuba_process_switch(plist[i_pid_to_place(pid_current)].scuba_dir);
    }

    if (sheduler_state == SHDLR_RUNN) {
        sheduler_state = SHDLR_ENBL;
    } else {
        sys_fatal("Sheduler is not running but sheduler is exiting");
    }

    sheduler_count--;
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

void i_optimize_shdlr_queue(void) {
    // TODO: separate all occurrences of each number as much as possible
    return;
}

int i_add_to_shdlr_queue(int pid, int priority) {
    if (shdlr_queue_length + priority > PROCESS_MAX * 10) {
        sys_error("Process queue is full");
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

void i_clean_killed(void) {
    need_clean = 0;
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].state == PROCESS_KILLED) {
            if (plist[i].pid == pid_current) {
                need_clean = 1;
                continue;
            }
            if (!plist[i].use_parent_dir)
                scuba_directory_destroy(plist[i].scuba_dir);
            free((void *) plist[i].esp_addr);
            plist[i].state = PROCESS_DEAD;
        }
    }
}

void i_refresh_tsleep_interact(void) {
    tsleep_interact = 0;
    for (int i = 0; i < tsleep_list_length; i++) {
        if (tsleep_list[i]->sleep_to < tsleep_interact || !tsleep_interact) {
            tsleep_interact = tsleep_list[i]->sleep_to;
        }
    }
}

void i_tsleep_awake(uint32_t ticks) {
    for (int i = 0; i < tsleep_list_length; i++) {
        if (tsleep_list[i]->sleep_to <= ticks) {
            if (tsleep_list[i]->state == PROCESS_TSLPING) {
                tsleep_list[i]->state = PROCESS_WAITING;
                i_add_to_shdlr_queue(tsleep_list[i]->pid, tsleep_list[i]->priority);
            } else {
                sys_fatal("Process in tsleep list is not in tsleep state");
            }

            tsleep_list[i] = tsleep_list[tsleep_list_length - 1];
            tsleep_list_length--;
            i--;
        }
    }
    i_refresh_tsleep_interact();
}

void i_remove_from_tsleep_list(uint32_t pid) {
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

void idle_process(void) {
    while (1) asm volatile("hlt");
}

/*********************
 * PUBLIC FUNCTIONS *
*********************/

int process_init(void) {
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
    kern_proc->priority = PROC_PRIORITY;
    kern_proc->comm = NULL;

    tsleep_interact = 0;
    pid_current = 0;
    need_clean = 0;

    sheduler_count = 0;

    shdlr_queue_length = 0;

    exit_codes = calloc(10 * sizeof(uint8_t));

    i_add_to_shdlr_queue(0, PROC_PRIORITY);

    kern_proc->scuba_dir = scuba_get_kernel_directory();

    // enable sheduler
    sheduler_state = SHDLR_ENBL;
    sheduler_disable_count = 0;

    // create idle process
    process_create(idle_process, 1, "idle", 0);
    plist[1].state = PROCESS_IDLETIME;

    return 0;
}

void test(void) { 
    // get return value
    uint32_t eax;
    asm volatile("movl %%eax, %0" : "=r" (eax));
    kprintf_serial("return value: %d\n", eax);
    force_exit_pid(process_get_pid(), eax);
}

int process_create(void (*func)(), int use_parent_dir, char *name, int nargs, ...) {
    int parent_place = i_pid_to_place(pid_current);
    int place = i_get_free_place();

    if (place == ERROR_CODE) {
        sys_warning("[create] Too many processes");
        return ERROR_CODE;
    }

    if (parent_place == ERROR_CODE) {
        sys_error("[create] Parent process not found");
        return ERROR_CODE;
    }

    static int pid_incrament = 0;
    pid_incrament++;

    if (pid_incrament % 10 == 9) {
        exit_codes = realloc_as_kernel(exit_codes, (pid_incrament + 10) * sizeof(uint8_t));
        mem_set(exit_codes + pid_incrament, 0, 10);
    }

    process_t *parent_proc = &plist[parent_place];
    process_t *new_proc = &plist[place];

    str_ncpy(new_proc->name, name, 63);

    new_proc->pid = pid_incrament;
    new_proc->ppid = pid_current;

    new_proc->use_parent_dir = use_parent_dir;

    new_proc->state = PROCESS_FSLPING;
    new_proc->priority = PROC_PRIORITY;

    new_proc->sleep_to = 0;
    new_proc->run_time = 0;

    new_proc->comm = NULL;

    i_new_process(new_proc, func, parent_proc->regs.eflags, (uint32_t *) parent_proc->scuba_dir);

    if (use_parent_dir) {
        new_proc->scuba_dir = parent_proc->scuba_dir;
    } else {
        new_proc->scuba_dir = scuba_directory_create();
        scuba_directory_init(new_proc->scuba_dir);
    }

    // push arguments to the new process
    uint32_t *esp = (uint32_t *) new_proc->regs.esp;
    uint32_t *args = (uint32_t *) &nargs;
    esp -= nargs + 1;

    for (int i = 1; i <= nargs; i++) {
        esp[i] = args[i];
        kprintf_serial("arg %d: %x\n", i, esp[i]);
    }

    // push exit function pointer
    esp[0] = (uint32_t) &test;

    new_proc->regs.esp = (uint32_t) esp;

    return pid_incrament;
}


int process_sleep(uint32_t pid, uint32_t ms) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[sleep] pid %d not found", pid);
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_IDLETIME) {
        sys_warning("[sleep] Can't interact with idle process");
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_FSLPING) {
        sys_warning("[sleep] pid %d already sleeping", pid);
        return ERROR_CODE;
    }

    if (plist[place].state >= PROCESS_KILLED) {
        sys_warning("[sleep] pid %d already dead", pid);
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


int process_wakeup(uint32_t pid) {   // TODO: sleep to exit gestion
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[wakeup] pid %d not found", pid);
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_IDLETIME) {
        sys_warning("[wakeup] Can't interact with idle process");
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_DEAD) {
        sys_warning("[wakeup] pid %d already dead", pid);
        return ERROR_CODE;
    }

    if (!(plist[place].state == PROCESS_FSLPING || plist[place].state == PROCESS_TSLPING)) {
        sys_warning("[wakeup] pid %d not sleeping", pid);
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


int process_handover(uint32_t pid) {
    int place = i_pid_to_place(pid);
    int current_place = i_pid_to_place(pid_current);

    if (place < 0) {
        sys_warning("[handover] pid %d not found", pid);
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_IDLETIME) {
        sys_warning("[handover] Can't interact with idle process");
        return ERROR_CODE;
    }

    if (plist[current_place].state == PROCESS_IDLETIME) {
        process_wakeup(pid);
        return 0;
    }

    if (plist[place].state == PROCESS_DEAD) {
        sys_warning("[handover] pid %d already dead", pid);
        return ERROR_CODE;
    }

    if (!(plist[place].state == PROCESS_FSLPING || plist[place].state == PROCESS_TSLPING)) {
        sys_warning("[handover] pid %d not sleeping", pid);
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


int process_kill(uint32_t pid) {
    if (pid == 0) {
        sys_warning("[kill] Can't kill kernel (^_^ )");
        return ERROR_CODE;
    }

    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[kill] pid %d not found", pid);
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_IDLETIME) {
        sys_warning("[kill] Can't interact with idle process");
        return ERROR_CODE;
    }

    if (plist[place].state >= PROCESS_KILLED) {
        sys_warning("[kill] pid %d already dead", pid);
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

/************************
 * SCHEUDLER FUNCTIONS *
************************/

void process_set_sheduler(int state) {
    // disable sheduler
    if (!state) {
        sheduler_disable_count++;
        if (sheduler_state == SHDLR_ENBL) {
            sheduler_state = SHDLR_DISL;
        }
    }

    // enable sheduler
    else if (state) {
        sheduler_disable_count--;
        if (sheduler_state == SHDLR_DISL && !sheduler_disable_count) {
            sheduler_state = SHDLR_ENBL;
        }
    }
}

void schedule(uint32_t ticks) {
    if (sheduler_count) return;
    sheduler_count++;

    if (sheduler_state != SHDLR_ENBL) {
        sheduler_count--;
        return;
    }

    sheduler_state = SHDLR_RUNN;

    // tick perfect backup verification system
    if (sheduler_count > 1) {
        sys_fatal("Multiple sheduler security fail");
        return;
    }

    if (tsleep_interact && tsleep_interact <= ticks && ticks) {
        i_tsleep_awake(ticks);
    }

    if (ticks % SCHEDULER_EVRY) {
        i_end_sheduler();
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

    uint32_t pid;

    if (shdlr_queue_length == 0) {
        pid = 1;    // idle process
    } else {
        pid = shdlr_queue[shdlr_queue_index];
    }

    if (pid != pid_current) {
        i_process_switch(pid_current, pid, ticks);
    } else {
        i_end_sheduler();
    }
}

/************************
 * GET / SET FUNCTIONS *
************************/

int process_set_priority(uint32_t pid, int priority) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[set_priority] pid %d not found", pid);
        return 1;
    }

    if (priority > 10 || priority < 1) {
        sys_warning("[set_priority] priority %d is not valid", priority);
        return 1;
    }

    plist[place].priority = priority;

    if (plist[place].state < PROCESS_TSLPING) {
        process_disable_sheduler();

        i_remove_from_shdlr_queue(pid);
        i_add_to_shdlr_queue(pid, priority);

        process_enable_sheduler();
    }

    return 0;
}

uint32_t process_get_pid(void) {
    return pid_current;
}

int process_set_comm(uint32_t pid, comm_struct_t *comm) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[set_comm] pid %d not found", pid);
        return 1;
    }

    plist[place].comm = comm;
    return 0;
}

comm_struct_t *process_get_comm(uint32_t pid) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[get_comm] pid %d not found", pid);
        return NULL;
    }

    return plist[place].comm;
}

int process_generate_pid_list(uint32_t *list, int max) {
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

scuba_directory_t *process_get_directory(uint32_t pid) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[get_directory] pid %d not found", pid);
        return 0;
    }

    return plist[place].scuba_dir;
}

int process_set_return(uint32_t pid, uint32_t ret) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[set_return] pid %d not found", pid);
        return 1;
    }

    exit_codes[pid] = ret;
    return 0;
}

uint32_t process_get_info(uint32_t pid, int info_id) {
    int place = i_pid_to_place(pid);

    if (info_id == PROCESS_INFO_EXIT_CODE) {
        if (place < 0)
            return 0;
        return exit_codes[pid];
    }

    if (info_id == PROCESS_INFO_STATE) {
        if (place < 0)
            return PROCESS_DEAD;
        return plist[place].state;
    }

    if (place < 0) {
        sys_warning("[get_info] pid %d not found", pid);
        return 1;
    }

    switch (info_id) {
        case PROCESS_INFO_PPID:
            return plist[place].ppid;
        case PROCESS_INFO_PRIORITY:
            return plist[place].priority;
        case PROCESS_INFO_SLEEP_TO:
            return plist[place].sleep_to;
        case PROCESS_INFO_RUN_TIME:
            return plist[place].run_time;
        case PROCESS_INFO_NAME:
            return (uint32_t) plist[place].name;
        default:
            sys_warning("[get_info] info_id %d not found", info_id);
            return 1;
    }
}
