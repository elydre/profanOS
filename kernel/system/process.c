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

#include <kernel/snowflake.h>
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

process_t *plist;

process_t **g_tsleep_list;
uint32_t g_tsleep_interact;
int g_tsleep_list_length;

int g_shdlr_queue_length;
int *g_shdlr_queue;

uint8_t g_scheduler_state = SHDLR_DEAD;
uint8_t g_need_clean;
int g_scheduler_disable_count;

uint32_t g_pid_incrament;
uint32_t g_pid_current;
uint8_t *g_exit_codes;

/**************************
 *                       *
 *  INTERNAL FUNCTIONS   *
 *                       *
**************************/

void i_new_process(process_t *process, void (*func)(), uint32_t flags, uint32_t *pagedir) {
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

void i_end_scheduler(void) {
    if (g_scheduler_state == SHDLR_RUNN) {
        g_scheduler_state = g_scheduler_disable_count ? SHDLR_DISL : SHDLR_ENBL;
    } else {
        sys_fatal("Scheduler is not running but scheduler is exiting");
    }
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

    g_pid_current = to_pid;

    process_asm_switch(&proc1->regs, &proc2->regs);
}

void i_optimize_shdlr_queue(void) {
    // TODO: separate all occurrences of each number as much as possible
    return;
}

int i_add_to_g_shdlr_queue(int pid, int priority) {
    if (g_shdlr_queue_length + priority > PROCESS_MAX * 10) {
        sys_error("Process queue is full");
        return ERROR_CODE;
    }

    for (int i = 0; i < priority; i++) {
        g_shdlr_queue[g_shdlr_queue_length + i] = pid;
    }

    g_shdlr_queue_length += priority;

    // i_optimize_shdlr_queue();

    return 0;
}

int i_remove_from_shdlr_queue(int pid) {
    // remove all occurences of pid from g_shdlr_queue
    for (int i = 0; i < g_shdlr_queue_length; i++) {
        if (g_shdlr_queue[i] == pid) {
            g_shdlr_queue[i] = g_shdlr_queue[g_shdlr_queue_length - 1];
            g_shdlr_queue_length--;
            i--;
        }
    }

    // i_optimize_shdlr_queue();

    return 0;
}

void i_clean_killed(void) {
    g_need_clean = 0;
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (plist[i].state == PROCESS_KILLED) {
            if (plist[i].pid == g_pid_current) {
                g_need_clean = 1;
                continue;
            }
            scuba_directory_destroy(plist[i].scuba_dir);
            plist[i].state = PROCESS_DEAD;
        }
    }
}

void i_refresh_tsleep_interact(void) {
    g_tsleep_interact = 0;
    for (int i = 0; i < g_tsleep_list_length; i++) {
        if (g_tsleep_list[i]->sleep_to < g_tsleep_interact || !g_tsleep_interact) {
            g_tsleep_interact = g_tsleep_list[i]->sleep_to;
        }
    }
}

void i_tsleep_awake(uint32_t ticks) {
    for (int i = 0; i < g_tsleep_list_length; i++) {
        if (g_tsleep_list[i]->sleep_to > ticks)
            continue;
        if (g_tsleep_list[i]->state == PROCESS_TSLPING) {
            g_tsleep_list[i]->state = PROCESS_WAITING;
            i_add_to_g_shdlr_queue(g_tsleep_list[i]->pid, g_tsleep_list[i]->priority);
        } else {
            sys_fatal("Process in tsleep list is not in tsleep state");
        }

        g_tsleep_list[i] = g_tsleep_list[g_tsleep_list_length - 1];
        g_tsleep_list_length--;
        i--;
    }
    i_refresh_tsleep_interact();
}

void i_remove_from_g_tsleep_list(uint32_t pid) {
    for (int i = 0; i < g_tsleep_list_length; i++) {
        if (g_tsleep_list[i]->pid == pid) {
            g_tsleep_list[i] = g_tsleep_list[g_tsleep_list_length - 1];
            g_tsleep_list_length--;
            i--;
        }
    }
    i_refresh_tsleep_interact();
}

void i_process_final_jump(void) {
    // get return value
    uint32_t eax;
    asm volatile("movl %%eax, %0" : "=r" (eax));
    force_exit_pid(g_pid_current, eax, 1);
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
    g_shdlr_queue = calloc(sizeof(int) * PROCESS_MAX * 10);
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
    kern_proc->priority = PROC_PRIORITY;
    kern_proc->comm = NULL;

    g_tsleep_interact = 0;
    g_pid_current = 0;
    g_need_clean = 0;

    g_pid_incrament = 0;

    g_shdlr_queue_length = 0;

    g_exit_codes = calloc(10 * sizeof(uint8_t));

    i_add_to_g_shdlr_queue(0, PROC_PRIORITY);

    kern_proc->scuba_dir = scuba_get_kernel_directory();

    // enable scheduler
    g_scheduler_state = SHDLR_ENBL;
    g_scheduler_disable_count = 0;

    // create idle process
    process_create(idle_process, 0, "idle", 0, NULL);
    plist[1].state = PROCESS_IDLETIME;

    return 0;
}


int process_create(void *func, int copy_page, char *name, int nargs, uint32_t *args) {
    int parent_place = i_pid_to_place(g_pid_current);
    int place = i_get_free_place();

    if (place == ERROR_CODE) {
        sys_warning("[create] Too many processes");
        return ERROR_CODE;
    }

    if (parent_place == ERROR_CODE) {
        sys_error("[create] Parent process not found");
        return ERROR_CODE;
    }

    g_pid_incrament++;

    if (g_pid_incrament % 10 == 9) {
        g_exit_codes = realloc_as_kernel(g_exit_codes, (g_pid_incrament + 10) * sizeof(uint8_t));
        mem_set(g_exit_codes + g_pid_incrament, 0, 10);
    }

    process_t *parent_proc = &plist[parent_place];
    process_t *new_proc = &plist[place];

    str_ncpy(new_proc->name, name, 63);

    new_proc->pid = g_pid_incrament;
    new_proc->ppid = g_pid_current;

    new_proc->state = PROCESS_FSLPING;
    new_proc->priority = PROC_PRIORITY;

    new_proc->sleep_to = 0;
    new_proc->run_time = 0;

    new_proc->comm = NULL;

    void *phys_stack;

    if (copy_page) {
        new_proc->scuba_dir = scuba_directory_copy(parent_proc->scuba_dir);
        phys_stack = scuba_get_phys(new_proc->scuba_dir, (void *) PROC_ESP_ADDR);
    } else {
        new_proc->scuba_dir = scuba_directory_inited();
        phys_stack = scuba_create_virtual(new_proc->scuba_dir, (void *) PROC_ESP_ADDR, PROC_ESP_SIZE / 0x1000);
    }

    i_new_process(new_proc, func, parent_proc->regs.eflags, (uint32_t *) new_proc->scuba_dir);

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
    esp[0] = (uint32_t) i_process_final_jump; // todo: check if this is the right address

    new_proc->regs.esp = PROC_ESP_ADDR + PROC_ESP_SIZE - (nargs + 1) * sizeof(uint32_t);

    return g_pid_incrament;
}


int process_fork(registers_t *regs) {
    int new_pid = process_create(NULL, 1, "forked", 0, NULL);

    if (new_pid == ERROR_CODE) {
        return ERROR_CODE;
    }

    process_t *new_proc = &plist[i_pid_to_place(new_pid)];

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

    process_wakeup(new_pid);

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

    if (ms == 0) {
        plist[place].state = PROCESS_FSLPING;
    } else {
        plist[place].state = PROCESS_TSLPING;
        // convert ms to ticks
        plist[place].sleep_to = timer_get_ticks() + (ms * 1000 / RATE_TIMER_TICK);
        g_tsleep_list[g_tsleep_list_length] = &plist[place];
        g_tsleep_list_length++;
        i_refresh_tsleep_interact();
    }

    i_remove_from_shdlr_queue(pid);

    if (pid == g_pid_current) {
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
    i_add_to_g_shdlr_queue(pid, plist[place].priority);

    i_refresh_tsleep_interact();

    return 0;
}


int process_handover(uint32_t pid) {
    int place = i_pid_to_place(pid);
    int current_place = i_pid_to_place(g_pid_current);

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

    if (plist[place].state == PROCESS_TSLPING) {
        i_remove_from_g_tsleep_list(pid);
    }

    plist[place].state = PROCESS_WAITING;
    i_add_to_g_shdlr_queue(pid, plist[place].priority);

    if (plist[current_place].state == PROCESS_TSLPING) {
        i_remove_from_g_tsleep_list(g_pid_current);
    }

    plist[current_place].state = PROCESS_FSLPING;
    i_remove_from_shdlr_queue(g_pid_current);

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
    i_remove_from_shdlr_queue(pid);

    g_need_clean = 1;

    if (pid == g_pid_current) {
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
        }
    }

    return 0;
}

void schedule(uint32_t ticks) {
    if (ticks && g_scheduler_state != SHDLR_ENBL) {
        return;
    }

    g_scheduler_state = SHDLR_RUNN;

    if (g_tsleep_interact && g_tsleep_interact <= ticks && ticks) {
        i_tsleep_awake(ticks);
    }

    if (ticks % SCHEDULER_EVRY) {
        i_end_scheduler();
        return;
    }

    if (g_need_clean) {
        i_clean_killed();
    }

    static int g_shdlr_queue_index = 0;
    g_shdlr_queue_index++;

    if (g_shdlr_queue_index >= g_shdlr_queue_length) {
        g_shdlr_queue_index = 0;
    }

    uint32_t pid;

    if (g_shdlr_queue_length == 0) {
        pid = 1;    // idle process
    } else {
        pid = g_shdlr_queue[g_shdlr_queue_index];
    }

    if (pid != g_pid_current) {
        i_process_switch(g_pid_current, pid, ticks);
    } else {
        i_end_scheduler();
    }
}

/**************************
 *                       *
 *  GET / SET FUNCTIONS  *
 *                       *
**************************/

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
        i_remove_from_shdlr_queue(pid);
        i_add_to_g_shdlr_queue(pid, priority);
    }

    return 0;
}

uint32_t process_get_pid(void) {
    return g_pid_current;
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

    if (pid == g_pid_current) {
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
        case PROCESS_INFO_PRIORITY:
            return plist[place].priority;
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
