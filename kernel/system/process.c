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

#define SHDLR_COOP 0
#define SHDLR_PEMP 1

process_t  *g_plist;                 // main process list
process_t  *g_proc_current = NULL;   // running process pointer
uint32_t    g_pid_incrament;         // pid of the last created process
uint32_t    g_last_switch;           // last scheduler switch time

process_t **g_tsleep_list;           // sleeping processes
uint32_t    g_tsleep_list_length;    // number of sleeping processes
uint32_t    g_tsleep_interact;       // time when the next sleeping process will awake

process_t **g_shdlr_queue;              // scheduler queue
uint32_t    g_shdlr_queue_length;       // scheduler queue length
int         g_shdlr_mode = SHDLR_COOP;  // scheduler mode (cooperative / preemptive)
uint8_t     g_need_clean;               // need to clean killed processes

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
    process->regs.esp = PROC_ESP_ADDR + PROC_ESP_SIZE - 4;
}

static int i_get_free_place(int pid) {
    int ideal = pid % PROCESS_MAX;

    for (int i = PROCESS_MAX + ideal; i > ideal; i--) {
        if (g_plist[i % PROCESS_MAX].state == PROC_STATE_FRE) {
            return i % PROCESS_MAX;
        }
    }

    return ERROR_CODE;
}

static int i_pid_to_place(uint32_t pid) {
    int ideal = pid % PROCESS_MAX;

    for (int i = PROCESS_MAX + ideal; i > ideal; i--) {
        if (g_plist[i % PROCESS_MAX].pid == pid && g_plist[i % PROCESS_MAX].state != PROC_STATE_FRE) {
            return i % PROCESS_MAX;
        }
    }

    return ERROR_CODE;
}

// used in switch.asm
void i_end_scheduler(void) {
    if (IN_KERNEL != g_proc_current->in_kernel) {
        if (g_proc_current->in_kernel) {
            sys_fatal("ENTER KERNEL, pid %d\n", g_proc_current->pid);
        } else {
            kprintf_serial("EXIT KERNEL, pid %d\n", g_proc_current->pid);
            sys_exit_kernel(0);
        }
    }
}

static void i_process_switch(process_t *proc1, process_t *proc2) {
    // this function is called when a process is
    // switched so we don't need security checks

    if (proc1->state == PROC_STATE_RUN) {
        proc1->state = PROC_STATE_INQ;
    }

    if (proc2->state != PROC_STATE_IDL) {
        proc2->state = PROC_STATE_RUN;
    }

    g_proc_current = proc2;
    proc1->in_kernel = 1;

    process_asm_switch(&proc1->regs, &proc2->regs);
}

static int i_add_to_shdlr_queue(process_t *proc) {
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

static void i_free_process(process_t *proc) {
    if (!proc->scuba_dir)
        return;

    mem_free_all(proc->pid, 0);

    scuba_dir_destroy(proc->scuba_dir);
    proc->scuba_dir = NULL;

    int parent_place = i_pid_to_place(proc->ppid);

    if (parent_place < 0 || g_plist[parent_place].state == PROC_STATE_ZMB) {
        proc->state = PROC_STATE_FRE;
    }
}

static void i_clean_killed(void) {
    g_need_clean = 0;
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (g_plist[i].state != PROC_STATE_ZMB || !g_plist[i].scuba_dir)
            continue;
        if (g_plist[i].pid != g_proc_current->pid) {
            i_free_process(g_plist + i);
        } else {
            g_need_clean = 1;
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
        if (g_tsleep_list[i]->state == PROC_STATE_SLP) {
            g_tsleep_list[i]->state = PROC_STATE_INQ;
            i_add_to_shdlr_queue(g_tsleep_list[i]);
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
    process_kill(g_proc_current->pid, eax);
}

/**************************
 *                       *
 *     IDLE PROCESS      *
 *                       *
**************************/

void idle_process(void) {
    // used to keep the CPU busy when no process is running
    // be careful: it runs out of kernel mode !
    while (1) asm volatile("hlt");
}

/**************************
 *                       *
 *   PUBLIC FUNCTIONS    *
 *                       *
**************************/

int process_init(void) {
    g_plist = calloc(sizeof(process_t) * PROCESS_MAX);
    g_shdlr_queue = calloc(sizeof(process_t *) * PROCESS_MAX);
    g_tsleep_list = calloc(sizeof(process_t *) * PROCESS_MAX);

    process_t *kern_proc = &g_plist[0];

    // Get EFLAGS and CR3
    asm volatile (
        "movl %%cr3, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (kern_proc->regs.cr3)
        :: "eax"
    );

    asm volatile (
        "pushfl\n\t"
        "movl (%%esp), %%eax\n\t"
        "movl %%eax, %0\n\t"
        "popfl"
        : "=m"(kern_proc->regs.eflags)
        :: "eax"
    );

    str_cpy(kern_proc->name, "kernel");
    kern_proc->state = PROC_STATE_RUN;

    kern_proc->ppid = 0; // it's worse than inbreeding!
    kern_proc->pid = 0;

    g_proc_current = kern_proc;

    g_tsleep_interact = 0;
    g_last_switch = 0;
    g_need_clean = 0;

    g_pid_incrament = 0;

    g_shdlr_queue_length = 0;

    i_add_to_shdlr_queue(kern_proc);

    kern_proc->scuba_dir = scuba_get_kernel_dir();

    // create idle process
    process_create(idle_process, 0, 0, NULL);
    str_cpy(g_plist[1].name, "idle");

    g_plist[1].state = PROC_STATE_IDL;
    g_plist[1].in_kernel = 0;

    return 0;
}


int process_create(void *func, int copy_page, int nargs, uint32_t *args) {
    int place = i_get_free_place(g_pid_incrament + 1);

    if (place == ERROR_CODE) {
        sys_warning("[create] Too many processes");
        return ERROR_CODE;
    }

    g_pid_incrament++;

    process_t *new_proc = g_plist + place;

    mem_set(new_proc, 0, sizeof(process_t));

    str_cpy(new_proc->name, "child");

    new_proc->pid = g_pid_incrament;
    new_proc->ppid = g_proc_current->pid;

    new_proc->in_kernel = (uint32_t) func < 0x200000;
    new_proc->state = PROC_STATE_SLP;

    void *phys_stack;

    if (copy_page) {
        new_proc->scuba_dir = scuba_dir_copy(g_proc_current->scuba_dir, new_proc->pid);
        phys_stack = scuba_get_phys(new_proc->scuba_dir, (void *) PROC_ESP_ADDR);
    } else {
        new_proc->scuba_dir = scuba_dir_inited(g_proc_current->scuba_dir, new_proc->pid);
        phys_stack = scuba_create_virtual(new_proc->scuba_dir, (void *) PROC_ESP_ADDR, PROC_ESP_SIZE / 0x1000);
    }

    i_new_process(new_proc, func, g_proc_current->regs.eflags, (uint32_t *) new_proc->scuba_dir);

    if (func == NULL) {
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
    int new_pid = process_create(NULL, 1, 0, NULL);

    if (new_pid == ERROR_CODE) {
        return ERROR_CODE;
    }

    process_t *new_proc = &g_plist[i_pid_to_place(new_pid)];

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

    if (g_plist[place].state == PROC_STATE_IDL) {
        sys_warning("[sleep] Can't interact with idle process");
        return ERROR_CODE;
    }

    if (g_plist[place].state == PROC_STATE_ZMB) {
        sys_warning("[sleep] pid %d already dead", pid);
        return ERROR_CODE;
    }

    if (ms == 0) {
        if (pid == g_proc_current->pid)
            schedule(0);
        return 0;
    }

    if (g_plist[place].state == PROC_STATE_SLP && g_plist[place].sleep_to) {
        i_remove_from_g_tsleep_list(pid);
    }

    g_plist[place].state = PROC_STATE_SLP;
    if (ms == (uint32_t) -1) {
        g_plist[place].sleep_to = 0;
    } else {
        // convert ms to ticks
        g_plist[place].sleep_to = TIMER_TICKS + (ms * 1000 / RATE_TIMER_TICK);
        g_tsleep_list[g_tsleep_list_length] = g_plist + place;
        g_tsleep_list_length++;
        i_refresh_tsleep_interact();
    }

    i_remove_from_shdlr_queue(&g_plist[place]);

    if (pid == g_proc_current->pid) {
        schedule(0);
    }

    return 0;
}


int process_wakeup(uint32_t pid, int handover) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[wakeup] pid %d not found", pid);
        return ERROR_CODE;
    }

    if (g_plist[place].state == PROC_STATE_IDL) {
        sys_warning("[wakeup] Can't interact with idle process");
        return ERROR_CODE;
    }

    if (g_plist[place].state == PROC_STATE_ZMB) {
        sys_warning("[wakeup] pid %d already dead", pid);
        return ERROR_CODE;
    }

    if (g_plist[place].state != PROC_STATE_SLP) {
        sys_warning("[wakeup] pid %d not sleeping", pid);
        return ERROR_CODE;
    }

    if (g_plist[place].sleep_to) {
        i_remove_from_g_tsleep_list(pid);
    }

    g_plist[place].state = PROC_STATE_INQ;
    g_plist[place].sleep_to = 0;
    g_plist[place].wait_pid = 0;

    i_add_to_shdlr_queue(&g_plist[place]);
    i_refresh_tsleep_interact();

    if (handover) {
        g_proc_current->state = PROC_STATE_SLP;
        g_proc_current->wait_pid = pid;
        g_proc_current->sleep_to = 0;

        i_remove_from_shdlr_queue(g_proc_current);
        schedule(0);
    }

    return 0;
}

int process_kill(uint32_t pid, uint8_t retcode) {
    if (pid == 0) {
        sys_warning("[kill] Can't kill kernel (^_^ )");
        return ERROR_CODE;
    }

    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[kill] pid %d not found", pid);
        return ERROR_CODE;
    }

    if (g_plist[place].state == PROC_STATE_IDL) {
        sys_warning("[kill] Can't interact with idle process");
        return ERROR_CODE;
    }

    if (g_plist[place].state == PROC_STATE_ZMB) {
        sys_warning("[kill] pid %d already dead", pid);
        return ERROR_CODE;
    }

    if (g_plist[place].state == PROC_STATE_SLP && g_plist[place].sleep_to) {
        i_remove_from_g_tsleep_list(pid);
    }

    g_plist[place].state = PROC_STATE_ZMB;
    g_plist[place].retcode = retcode;
    g_plist[place].wait_pid = 0;

    i_remove_from_shdlr_queue(&g_plist[place]);

    g_need_clean = 1;

    for (int i = 0; i < PROCESS_MAX; i++) {
        if (i == place)
            continue;

        // wake up parent process that is waiting for this one
        if (g_plist[i].wait_pid == (int) pid ||
           (g_plist[i].wait_pid == -1 && g_plist[i].pid == g_plist[place].ppid)) {
            g_plist[i].wait_pid = -pid;
            process_wakeup(g_plist[i].pid, 0);
        }

        // kill children
        if (g_plist[i].ppid == pid && g_plist[i].state != PROC_STATE_FRE) {
            if (g_plist[i].state == PROC_STATE_ZMB) {
                i_free_process(g_plist + i);
                g_plist[i].state = PROC_STATE_FRE;
                continue;
            }
            process_kill(g_plist[i].pid, 0);
        }
    }

    if (pid == g_proc_current->pid) {
        schedule(0);
    }

    return 0;
}

int process_wait(int pid, uint8_t *retcode, int block) {
    if (pid < 1)
        pid = -1;

    if (pid < 0) {
        for (int i = 0; i < PROCESS_MAX; i++) {
            if (g_plist[i].state == PROC_STATE_ZMB && g_plist[i].ppid == g_proc_current->pid && g_plist[i].retcode >= 0) {
                if (retcode)
                    *retcode = (uint8_t) g_plist[i].retcode;
                pid = g_plist[i].pid;
                i_free_process(g_plist + i);
                g_plist[i].state = PROC_STATE_FRE;
                return pid;
            }
        }

        // check for children
        for (int i = 0; i < PROCESS_MAX; i++) {
            if (g_plist[i].ppid == g_proc_current->pid && g_plist[i].state > PROC_STATE_ZMB)
                break;
            if (i == PROCESS_MAX - 1) {
                return -1;
            }
        }
    } else {
        int child_place = i_pid_to_place(pid);

        if (child_place < 0 || g_plist[child_place].ppid != g_proc_current->pid) {
            return ERROR_CODE;
        }

        if (g_plist[child_place].state == PROC_STATE_ZMB) {
            if (retcode)
                *retcode = (uint8_t) g_plist[child_place].retcode;
            i_free_process(g_plist + child_place);
            g_plist[child_place].state = PROC_STATE_FRE;
            return pid;
        }
    }

    if (!block) {
        return 0;
    }

    g_proc_current->state = PROC_STATE_SLP;
    g_proc_current->wait_pid = pid < 0 ? -1 : pid;
    g_proc_current->sleep_to = 0;

    i_remove_from_shdlr_queue(g_proc_current);
    schedule(0);

    // process is awake again

    pid = g_proc_current->wait_pid;
    g_proc_current->wait_pid = 0;

    if (pid == -1 || pid > 0) {
        return ERROR_CODE;
    }

    for (int i = 0; i < PROCESS_MAX; i++) {
        if (g_plist[i].state == PROC_STATE_ZMB && g_plist[i].ppid == g_proc_current->pid && g_plist[i].retcode >= 0) {
            if (retcode)
                *retcode = (uint8_t) g_plist[i].retcode;
            pid = g_plist[i].pid;
            i_free_process(g_plist + i);
            g_plist[i].state = PROC_STATE_FRE;
            return pid;
        }
    }

    sys_fatal("Process woken up by death of %d but it is not found", pid);
    return ERROR_CODE;
}

/**************************
 *                       *
 *  SCHEUDLER FUNCTIONS  *
 *                       *
**************************/

void schedule(uint32_t ticks) {

    if (ticks == 0) {   // manual schedule
        ticks = TIMER_TICKS;
    }

    else if (ticks - g_last_switch < SCHEDULER_EVRY &&
            (g_proc_current->state != PROC_STATE_IDL || g_tsleep_interact > ticks)) {
        return;
    }

    if (g_tsleep_interact && g_tsleep_interact <= ticks) {
        i_tsleep_awake(ticks);
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
        proc = &g_plist[1]; // idle process
    } else {
        proc = g_shdlr_queue[g_shdlr_queue_index];
    }

    g_proc_current->run_time += ticks - g_last_switch;
    g_last_switch = ticks;

    if (proc == g_proc_current) {
        return;
    }

    i_process_switch(g_proc_current, proc);
}

void schedule_if_needed(void) {
    if (TIMER_TICKS - g_last_switch > SCHEDULER_EVRY) {
        kprintf_serial("FORCE SCHEDULE\n");
        schedule(0);
    }
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

int process_list_all(uint32_t *list, int max) {
    int i = 0;
    for (int j = 0; j < PROCESS_MAX && i < max; j++) {
        if (g_plist[j].state != PROC_STATE_FRE) {
            list[i++] = g_plist[j].pid;
        }
    }
    return i;
}

scuba_dir_t *process_get_dir(uint32_t pid) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[get_directory] pid %d not found", pid);
        return 0;
    }

    return g_plist[place].scuba_dir;
}

void process_switch_directory(uint32_t pid, scuba_dir_t *new_dir, int now) {
    int place = i_pid_to_place(pid);

    if (place < 0) {
        sys_warning("[set_directory] pid %d not found", pid);
        return;
    }

    scuba_dir_t *old_dir = g_plist[place].scuba_dir;

    g_plist[place].scuba_dir = new_dir;

    if (!now) return;

    if (pid == g_proc_current->pid) {
        scuba_switch(new_dir);
    }

    scuba_dir_destroy(old_dir);
}

int process_info(uint32_t pid, int info_id, void *ptr) {
    int place = i_pid_to_place(pid);

    if (info_id == PROC_INFO_STACK) {
        return PROC_ESP_ADDR;
    }

    if (info_id == PROC_INFO_STATE) {
        if (place < 0)
            return -1;
        return g_plist[place].state;
    }

    if (place < 0) {
        return -1;
    }

    switch (info_id) {
        case PROC_INFO_PPID:
            return g_plist[place].ppid;
        case PROC_INFO_SLEEP_TO:
            return g_plist[place].sleep_to;
        case PROC_INFO_RUN_TIME:
            if (pid == 0)
                return sys_get_kernel_time();
            return g_plist[place].run_time;
        case PROC_INFO_NAME:
            return (int) g_plist[place].name;
        case PROC_INFO_SET_NAME:
            if (!ptr)
                return -1;
            str_ncpy(g_plist[place].name, ptr, 63);
            return 0;
        default:
            return -1;
    }
}
