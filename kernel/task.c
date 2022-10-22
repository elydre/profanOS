#include <gui/vga.h>
#include <string.h>
#include <iolib.h>
#include <task.h>
#include <mem.h>

#define TASK_MAX 10

static task_t tasks[TASK_MAX + 1];
int current_pid, task_count;

/************************
 * INTERNAL FUNCTIONS *
************************/

void i_new_task(task_t *task, void (*main)(), uint32_t flags, uint32_t *pagedir, int pid) {
    task->regs.eax = 0;
    task->regs.ebx = 0;
    task->regs.ecx = 0;
    task->regs.edx = 0;
    task->regs.esi = 0;
    task->regs.edi = 0;
    task->regs.eflags = flags;
    task->regs.eip = (uint32_t) main;
    task->regs.cr3 = (uint32_t) pagedir;
    task->regs.esp = (uint32_t) mem_alloc(0x1000);
    task->pid = pid;
    task->isdead = 0;
}

int i_refresh_alive() {
    int decal = 0, nb_alive = 0;
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].isdead == 2) {
            decal++;
        }
        else {
            nb_alive++;
            if (decal > 0) tasks[i - decal] = tasks[i];
        }
    }
    task_count = nb_alive;
    return nb_alive;
}


void i_destroy_killed_tasks(int nb_alive) {
    for (int i = 1; i < nb_alive; i++) {
        if (tasks[i].isdead != 1) continue;
        mem_free_addr(tasks[i].regs.esp);
        tasks[i].isdead = 2;
    }
}

/***********************
 * EXTERNAL FUNCTIONS *
***********************/

void tasking_init() {
    static task_t mainTask;

    // Get EFLAGS and CR3
    asm volatile(
        "movl %%cr3, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (mainTask.regs.cr3)
        :: "%eax");

    asm volatile("pushfl\n\t"
        "movl (%%esp), %%eax\n\t"
        "movl %%eax, %0\n\t"
        "popfl"
        : "=m"(mainTask.regs.eflags)
        :: "%eax");

    mainTask.pid = 0;
    mainTask.isdead = 0;
    mainTask.gui_mode = 0;
    str_cpy(mainTask.name, "kernel");

    tasks[0] = mainTask;

    current_pid = 0;
    task_count = 1;
}

int task_create(void (*func)(), char * name) {
    int nb_alive = i_refresh_alive();
    if (task_count >= TASK_MAX) {
        sys_fatal("Cannot create task, too many tasks");
        return -1;
    }
    current_pid++;
    int pid = current_pid;
    task_t task, *mainTask;
    for (int i = 0; i < nb_alive; i++) {
        if (tasks[i].pid == 0) {
            mainTask = &tasks[i];
            break;
        }
    }

    task.gui_mode = vga_get_mode();
    str_cpy(task.name, name);

    i_new_task(&task, func, mainTask->regs.eflags, (uint32_t*) mainTask->regs.cr3, pid);
    tasks[nb_alive] = task;
    task_count++;
    return pid;
}

void yield(int target_pid) {
    int task_i, nb_alive = i_refresh_alive();

    for (task_i = 0; task_i < nb_alive; task_i++) {
        if (tasks[task_i].pid == target_pid) {
            tasks[TASK_MAX] = tasks[task_i];
            for (int i = task_i; i > 0; i--) {
                tasks[i] = tasks[i - 1];
            }
            tasks[0] = tasks[TASK_MAX];
            break;
        } else if (task_i == nb_alive - 1) {
            sys_error("Task not found in yield");
            return;
        }
    }

    vga_switch_mode(tasks[0].gui_mode);

    fskprint("$2yield: %d -> %d\n", tasks[1].pid, tasks[0].pid);

    task_switch(&tasks[1].regs, &tasks[0].regs);
    i_destroy_killed_tasks(nb_alive);
}

void task_kill_yield(int target_pid) {
    tasks[0].isdead = 1;
    yield(target_pid);
}

void task_kill(int target_pid) {
    int nb_alive = i_refresh_alive();
    for (int i = 0; i < nb_alive; i++) {
        if (tasks[i].pid == target_pid) {
            tasks[i].isdead = 1;
            i_destroy_killed_tasks(nb_alive);
            return;
        }
    }
    sys_error("Task not found in task_kill");
}

void task_update_gui_mode(int mode) {
    tasks[0].gui_mode = mode;
}

/*******************
 * GET FUNCTIONS *
*******************/

int task_get_current_pid() {
    return tasks[0].pid;
}

int task_get_next_pid() {
    task_get_alive();
    return tasks[1].pid;
}

int task_get_alive() {
    return i_refresh_alive();
}

int task_get_max() {
    return TASK_MAX;
}

void task_debug_print() {
    int nb_alive = i_refresh_alive();
    for (int i = 0; i < nb_alive; i++) {
        fskprint("$4Task %s: %d (%d, %d)\n", tasks[i].name, tasks[i].pid, tasks[i].isdead, tasks[i].gui_mode);
    }
}
