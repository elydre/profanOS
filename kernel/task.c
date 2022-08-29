#include <skprint.h>
#include <screen.h>
#include <string.h>
#include <task.h>
#include <mem.h>

#define TASK_MAX 5

static Task tasks[TASK_MAX];
int *task_count = 0;

static void other_main() {
    rainbow_print("Hello multitasking world!\n");
    yield(0);
    rainbow_print("Hello again!\n");
    kill_and_yield(0);
}

void create_task(Task *task, void (*main)(), uint32_t flags, uint32_t *pagedir, int pid) {
    task->regs.eax = 0;
    task->regs.ebx = 0;
    task->regs.ecx = 0;
    task->regs.edx = 0;
    task->regs.esi = 0;
    task->regs.edi = 0;
    task->regs.eflags = flags;
    task->regs.eip = (uint32_t) main;
    task->regs.cr3 = (uint32_t) pagedir;
    task->regs.esp = (uint32_t) alloc(0x1000);
    task->pid = pid;
    task->isdead = 0;
    char str[10];
    int_to_ascii(task->pid, str);
    mskprint(3, "Task created $1", str, "\n");
}

void init_tasking() {
    static Task mainTask;
    static Task otherTask;

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

    create_task(&otherTask, other_main, mainTask.regs.eflags, (uint32_t*)mainTask.regs.cr3, 1);

    tasks[0] = mainTask;
    tasks[1] = otherTask;

    *task_count = 2;

    kprint("Tasking initialized\n");
}

int refresh_alive() {
    int decal = 0; int nb_alive = 0;

    for (int i = 0; i < *task_count; i++) {
        if (tasks[i].isdead == 2) {
            decal++;
        }
        else {
            nb_alive++;
            if (decal > 0) tasks[i - decal] = tasks[i];
        }
    }
    *task_count = nb_alive;
    return nb_alive;
}

void powerfull_task(void (*main)(), int pid) {
    int nb_alive = refresh_alive();
    Task task;
    Task *mainTask;
    for (int i = 0; i < nb_alive; i++) {
        if (tasks[i].pid == 0) {
            mainTask = &tasks[i];
            break;
        }
    }
    create_task(&task, main, mainTask->regs.eflags, (uint32_t*)mainTask->regs.cr3, pid);
    tasks[nb_alive] = task;
    *task_count = nb_alive + 1;
}

void task_printer() {
    char str[10];
    int nb_alive = refresh_alive();
    int_to_ascii(nb_alive, str);
    mskprint(3, "$4task alive: $1", str, "\n");
    int_to_ascii(TASK_MAX, str);
    mskprint(3, "$4task max:   $1", str, "\n$4task list:  $7[");
    for (int i = 0; i < nb_alive; i++) {
        int_to_ascii(tasks[i].pid, str);
        ckprint(str, c_green);
        if (i != nb_alive - 1)  kprint(", ");
    }
    kprint("]\n");
}

void destroy_killed_tasks(int nb_alive) {
    char str[2];
    for (int i = 1; i < nb_alive; i++) {
        if (tasks[i].isdead == 1) {
            mskprint(3,"$4Task$1 ", str, " $4killed, free: ");
            if (free(tasks[i].regs.esp)) mskprint(1, "$1done!\n");
            else mskprint(1, "$3fail :(\n");
            tasks[i].isdead = 2;
            int_to_ascii(tasks[i].pid, str);
        }
    }
}

void yield(int target_pid) {
    int nb_alive = refresh_alive();
    char str_old[10], str_new[10];
    int_to_ascii(tasks[0].pid, str_old);
    int_to_ascii(target_pid, str_new);
    int task_i;

    for (task_i = 0; task_i < nb_alive; task_i++) {
        if (tasks[task_i].pid == target_pid) {
            tasks[TASK_MAX - 1] = tasks[0];
            tasks[0] = tasks[task_i];
            tasks[task_i] = tasks[TASK_MAX - 1];
            break;
        } else if (task_i == nb_alive - 1) {
            mskprint(3, "$4Task$1 ", str_new, " $4not found\n");
            return;
        }
    }

    mskprint(5, "$4yield from$1 ", str_old, " $4to$1 ", str_new, "\n");

    switch_task(&tasks[1].regs, &tasks[0].regs);

    destroy_killed_tasks(nb_alive);
}

void kill_and_yield(int target_pid) {
    ckprint("Task kill asked\n", c_dgrey);
    tasks[0].isdead = 1;
    yield(target_pid);
}
