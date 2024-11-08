/*****************************************************************************\
|   === process.c : 2024 ===                                                  |
|                                                                             |
|    Kernel process manager v2.1                                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/process.h>
#include <cpu/timer.h>
#include <minilib.h>
#include <system.h>

#define ERROR_CODE (-1)

#define SHDLR_ENBL 0    // scheduler enabled
#define SHDLR_RUNN 1    // scheduler running
#define SHDLR_DISL 2    // scheduler disabled
#define SHDLR_DEAD 3    // scheduler dead

#define SCHEDULER_EVRY (RATE_TIMER_TICK / RATE_SCHEDULER)

// main process list
process_t *plist;

// sleeping processes
uint32_t g_tsleep_list_length;
process_t **g_tsleep_list;

// first process to wake up
uint32_t g_tsleep_interact;

// scheduler queue
uint32_t g_shdlr_queue_length;
process_t **g_shdlr_queue;

// scheduler state
uint8_t g_scheduler_state = SHDLR_DEAD;
int g_scheduler_disable_count;

// need to clean killed processes
uint8_t g_need_clean;

// running process
process_t *g_proc_current = NULL;

// how many processes have been created
uint32_t g_pid_incrament;

// exit codes storage
uint8_t *g_exit_codes;

// last scheduler switch
uint32_t g_last_switch;

/**************************
 *                       *
 *  INTERNAL FUNCTIONS   *
 *                       *
**************************/

static void i_new_process(process_t *process, void (*func)(), uint32_t flags, uint32_t *pagedir) {
    process->regs.eax = 0;
    process->regs.ebx = 0;
    process->regs.ecx = 0;
    process->regs.edx = 0;
    process->regs.ebp = 0;
    process->regs.esi = 0;
    process->regs.edi = 0;
    process->regs.eflags = flags;
    process->regs.eip = (uint32_t) func;
    process->regs.cr3 = (uint32_t) pagedir;
    process->regs.esp = PROC_ESP_ADDR + PROC_ESP_SIZE;
}

static int i_get_free_place(void) {
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].state == PROCESS_DEAD) return i;
    }
    return ERROR_CODE;
}

static int i_pid_to_place(uint32_t pid) {
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].pid == pid) return i;
    }
    return ERROR_CODE;
}

// used in switch.asm
void i_end_scheduler(void) {
    if (IN_KERNEL != g_proc_current->in_kernel) {
        if (g_proc_current->in_kernel) {
            sys_entry_kernel(0);
        } else {
            sys_exit_kernel(0);
        }
    }

    if (g_scheduler_state == SHDLR_RUNN) {
        g_scheduler_state = g_scheduler_disable_count ? SHDLR_DISL : SHDLR_ENBL;
    } else {
        sys_fatal("Scheduler is not running but scheduler is exiting");
    }
}

static void i_process_switch(process_t *proc1, process_t *proc2) {
    // this function is called when a process is
    // switched so we don't need security checks

    if (proc1->state == PROCESS_RUNNING) {
        proc1->state = PROCESS_WAITING;
    }
    if (proc2->state != PROCESS_IDLETIME) {
        proc2->state = PROCESS_RUNNING;
    }

    g_proc_current = proc2;

    proc1->in_kernel = IN_KERNEL;

    process_asm_switch(&proc1->regs, &proc2->regs);
}

static int i_add_to_g_shdlr_queue(process_t *proc) {
    if (g_shdlr_queue_length >= PROCESS_MAX) {
        sys_fatal("Too many processes in scheduler queue");
        return 1;
    }

    for (uint32_t i = 0; i < g_shdlr_queue_length; i++) {
        if (g_shdlr_queue[i] == proc) {
            sys_fatal("Process already in scheduler queue");
            return 1;
        }
    }

    g_shdlr_queue[g_shdlr_queue_length++] = proc;

    return 0;
}

static int i_remove_from_shdlr_queue(process_t *proc) {
    for (uint32_t i = 0; i < g_shdlr_queue_length; i++) {
        if (g_shdlr_queue[i] == proc) {
            g_shdlr_queue[i] = g_shdlr_queue[g_shdlr_queue_length - 1];
            g_shdlr_queue_length--;
            return 0;
        }
    }
    return 0;
}

static void i_clean_killed(void) {
    g_need_clean = 0;
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].state == PROCESS_KILLED) {
            if (plist[i].pid == g_proc_current->pid) {
                g_need_clean = 1;
                continue;
            }
            scuba_directory_destroy(plist[i].scuba_dir);
            plist[i].state = PROCESS_DEAD;
        }
    }
}

static void i_refresh_tsleep_interact(void) {
    g_tsleep_interact = 0;
    for (uint32_t i = 0; i < g_tsleep_list_length; i++) {
        if (g_tsleep_list[i]->sleep_to < g_tsleep_interact || !g_tsleep_interact) {
            g_tsleep_interact = g_tsleep_list[i]->sleep_to;
        }
    }
}

static void i_tsleep_awake(uint32_t ticks) {
    for (uint32_t i = 0; i < g_tsleep_list_length; i++) {
        if (g_tsleep_list[i]->sleep_to > ticks)
            continue;
        if (g_tsleep_list[i]->state == PROCESS_TSLPING) {
            g_tsleep_list[i]->state = PROCESS_WAITING;
            i_add_to_g_shdlr_queue(g_tsleep_list[i]);
        } else {
            sys_fatal("Process in tsleep list is not in tsleep state");
        }

        g_tsleep_list[i] = g_tsleep_list[g_tsleep_list_length - 1];
        g_tsleep_list_length--;
        i--;
    }
    i_refresh_tsleep_interact();
}

static void i_remove_from_g_tsleep_list(uint32_t pid) {
    for (uint32_t i = 0; i < g_tsleep_list_length; i++) {
        if (g_tsleep_list[i]->pid == pid) {
            g_tsleep_list[i] = g_tsleep_list[g_tsleep_list_length - 1];
            g_tsleep_list_length--;
            i--;
        }
    }
    i_refresh_tsleep_interact();
}

static void i_process_final_jump(void) {
    // get return value
    uint32_t eax;
    asm volatile("movl %%eax, %0" : "=r" (eax));
    force_exit_pid(g_proc_current->pid, eax, 1);
}

/**************************
 *                       *
 *     IDLE PROCESS      *
 *                       *
**************************/

void idle_process(void) {
    while (1) asm volatile("hlt");
}

/**************************
 *                       *
 *   PUBLIC FUNCTIONS    *
 *                       *
**************************/

int process_init(void) {
    plist = calloc(sizeof(process_t) * PROCESS_MAX);
    g_shdlr_queue = calloc(sizeof(process_t *) * PROCESS_MAX);
    g_tsleep_list = calloc(sizeof(process_t *) * PROCESS_MAX);

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
    kern_proc->comm = NULL;

    g_proc_current = kern_proc;

    g_tsleep_interact = 0;
    g_last_switch = 0;
    g_need_clean = 0;

    g_pid_incrament = 0;

    g_shdlr_queue_length = 0;

    g_exit_codes = calloc(10 * sizeof(uint8_t));

    i_add_to_g_shdlr_queue(kern_proc);

    kern_proc->scuba_dir = scuba_get_kernel_directory();

    // enable scheduler
    g_scheduler_state = SHDLR_DISL;
    g_scheduler_disable_count = 1;

    // create idle process
    process_create(idle_process, 0, "idle", 0, NULL);
    plist[1].state = PROCESS_IDLETIME;
    plist[1].in_kernel = 0;

    return 0;
}


int process_create(void *func, int copy_page, char *name, int nargs, uint32_t *args) {
    int place = i_get_free_place();

    if (place == ERROR_CODE) {
        sys_warning("[create] Too many processes");
        return ERROR_CODE;
    }

    g_pid_incrament++;

    if (g_pid_incrament % 10 == 9) {
        g_exit_codes = realloc_as_kernel(g_exit_codes, (g_pid_incrament + 10) * sizeof(uint8_t));
        mem_set(g_exit_codes + g_pid_incrament, 0, 10);
    }

    process_t *new_proc = &plist[place];

    str_ncpy(new_proc->name, name, 63);

    new_proc->pid = g_pid_incrament;
    new_proc->ppid = g_proc_current->pid;

    new_proc->state = PROCESS_FSLPING;

    new_proc->sleep_to = 0;
    new_proc->run_time = 0;

    new_proc->comm = NULL;

    new_proc->in_kernel = (uint32_t) func < 0x200000;

    void *phys_stack;

    if (copy_page) {
        new_proc->scuba_dir = scuba_directory_copy(g_proc_current->scuba_dir, new_proc->pid);
        phys_stack = scuba_get_phys(new_proc->scuba_dir, (void *) PROC_ESP_ADDR);
    } else {
        new_proc->scuba_dir = scuba_directory_inited(new_proc->pid);
        phys_stack = scuba_create_virtual(new_proc->scuba_dir, (void *) PROC_ESP_ADDR, PROC_ESP_SIZE / 0x1000);
    }

    i_new_process(new_proc, func, g_proc_current->regs.eflags, (uint32_t *) new_proc->scuba_dir);

    if (func == NULL) {
        new_proc->regs.esp = PROC_ESP_ADDR + PROC_ESP_SIZE;
        return g_pid_incrament;
    }

    // push arguments to the new process
    uint32_t *esp = phys_stack + PROC_ESP_SIZE;
    esp -= nargs + 1;

    for (int i = 0; i < nargs; i++) {
        esp[i + 1] = args[i];
    }

    // push exit function pointer
    esp[0] = (uint32_t) i_process_final_jump; // TODO: check if this is the right address

    new_proc->regs.esp = PROC_ESP_ADDR + PROC_ESP_SIZE - (nargs + 1) * sizeof(uint32_t);

    return g_pid_incrament;
}


int process_fork(registers_t *regs) {
    int new_pid = process_create(NULL, 1, "forked", 0, NULL);

    if (new_pid == ERROR_CODE) {
        return ERROR_CODE;
    }

    process_t *new_proc = &plist[i_pid_to_place(new_pid)];

    new_proc->in_kernel = 0; // cannot call this without a syscall

    new_proc->regs.eax = regs->eax;
    new_proc->regs.ebx = regs->ebx;
    new_proc->regs.ecx = regs->ecx;
    new_proc->regs.edx = regs->edx;
    new_proc->regs.esi = regs->esi;
    new_proc->regs.edi = regs->edi;
    new_proc->regs.eip = regs->eip;
    new_proc->regs.esp = regs->esp + 20; // popa (interrupt.asm: isr_common_stub)
    new_proc->regs.ebp = regs->ebp;
    new_proc->regs.eflags = regs->eflags;

    return new_pid;
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

    if (plist[place].state == PROCESS_TSLPING) {
        i_remove_from_g_tsleep_list(pid);
    }

    if (pid == g_proc_current->pid && ms == 0) {
        schedule(0);
        return 0;
    }

    if (ms == (uint32_t) -1) {
        plist[place].state = PROCESS_FSLPING;
    } else {
        plist[place].state = PROCESS_TSLPING;
        // convert ms to ticks
        plist[place].sleep_to = timer_get_ticks() + (ms * 1000 / RATE_TIMER_TICK);
        g_tsleep_list[g_tsleep_list_length] = &plist[place];
        g_tsleep_list_length++;
        i_refresh_tsleep_interact();
    }

    i_remove_from_shdlr_queue(&plist[place]);

    if (pid == g_proc_current->pid) {
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

    if (plist[place].state == PROCESS_TSLPING) {
        i_remove_from_g_tsleep_list(pid);
    }

    plist[place].state = PROCESS_WAITING;
    i_add_to_g_shdlr_queue(&plist[place]);

    i_refresh_tsleep_interact();

    return 0;
}


int process_handover(uint32_t pid) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[handover] pid %d not found", pid);
        return ERROR_CODE;
    }

    if (plist[place].state == PROCESS_IDLETIME) {
        sys_warning("[handover] Can't interact with idle process");
        return ERROR_CODE;
    }

    if (g_proc_current->state == PROCESS_IDLETIME) {
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

    if (plist[place].state == PROCESS_TSLPING) {
        i_remove_from_g_tsleep_list(pid);
    }

    plist[place].state = PROCESS_WAITING;
    i_add_to_g_shdlr_queue(&plist[place]);

    if (g_proc_current->state == PROCESS_TSLPING) {
        i_remove_from_g_tsleep_list(g_proc_current->pid);
    }

    g_proc_current->state = PROCESS_FSLPING;
    i_remove_from_shdlr_queue(g_proc_current);

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

    if (plist[place].state == PROCESS_TSLPING) {
        i_remove_from_g_tsleep_list(pid);
    }

    plist[place].state = PROCESS_KILLED;
    i_remove_from_shdlr_queue(&plist[place]);

    g_need_clean = 1;

    if (pid == g_proc_current->pid) {
        schedule(0);
    }

    return 0;
}

/**************************
 *                       *
 *  SCHEUDLER FUNCTIONS  *
 *                       *
**************************/

int process_auto_schedule(int acitve) {
    if (!acitve) {
        g_scheduler_disable_count++;
        if (g_scheduler_state == SHDLR_ENBL) {
            g_scheduler_state = SHDLR_DISL;
        }
    } else {
        g_scheduler_disable_count--;
        if (g_scheduler_disable_count < 0)
            g_scheduler_disable_count = 0;
        if (g_scheduler_state == SHDLR_DISL && !g_scheduler_disable_count) {
            g_scheduler_state = SHDLR_ENBL;
            if (timer_get_ticks() - g_last_switch > SCHEDULER_EVRY) {
                schedule(0);
            }
        }
    }

    return 0;
}

void schedule(uint32_t ticks) {
    if (ticks && g_scheduler_state != SHDLR_ENBL) {
        return;
    }

    g_scheduler_state = SHDLR_RUNN;

    if (g_tsleep_interact && ticks && g_tsleep_interact <= ticks) {
        i_tsleep_awake(ticks);
    }

    if (ticks == 0) {   // manual schedule
        ticks = timer_get_ticks();
    } else if (ticks % SCHEDULER_EVRY) {
        g_scheduler_state = SHDLR_ENBL;
        return;
    }

    if (g_need_clean) {
        i_clean_killed();
    }

    static uint32_t g_shdlr_queue_index = 0;
    g_shdlr_queue_index++;

    if (g_shdlr_queue_index >= g_shdlr_queue_length) {
        g_shdlr_queue_index = 0;
    }

    process_t *proc;

    if (g_shdlr_queue_length == 0) {
        proc = &plist[1]; // idle process
    } else {
        proc = g_shdlr_queue[g_shdlr_queue_index];
    }

    g_proc_current->run_time += ticks - g_last_switch;
    g_last_switch = ticks;

    if (proc == g_proc_current) {
        g_scheduler_state = g_scheduler_disable_count ? SHDLR_DISL : SHDLR_ENBL;
        return;
    }

    i_process_switch(g_proc_current, proc);
}

/**************************
 *                       *
 *  GET / SET FUNCTIONS  *
 *                       *
**************************/

uint32_t process_get_pid(void) {
    if (!g_proc_current)
        return 0; // no initialized yet
    return g_proc_current->pid;
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

int process_list_all(uint32_t *list, int max) {
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

void process_switch_directory(uint32_t pid, scuba_directory_t *new_dir, int now) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[set_directory] pid %d not found", pid);
        return;
    }

    scuba_directory_t *old_dir = plist[place].scuba_dir;

    plist[place].scuba_dir = new_dir;

    if (!now) return;

    if (pid == g_proc_current->pid) {
        scuba_switch(new_dir);
    }

    scuba_directory_destroy(old_dir);
}

int process_set_return(uint32_t pid, uint32_t ret) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[set_return] pid %d not found", pid);
        return 1;
    }

    g_exit_codes[pid] = ret;
    return 0;
}

uint32_t process_get_info(uint32_t pid, int info_id) {
    int place = i_pid_to_place(pid);

    if (info_id == PROCESS_INFO_EXIT_CODE) {
        if (place < 0)
            return 0;
        return g_exit_codes[pid];
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
        case PROCESS_INFO_SLEEP_TO:
            return plist[place].sleep_to;
        case PROCESS_INFO_RUN_TIME:
            return plist[place].run_time;
        case PROCESS_INFO_NAME:
            return (uint32_t) plist[place].name;
        case PROCESS_INFO_STACK:
            return PROC_ESP_ADDR;
        default:
            sys_warning("[get_info] info_id %d not found", info_id);
            return 1;
    }
}
