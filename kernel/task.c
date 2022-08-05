#include "task.h"
#include "../drivers/screen.h"
#include "../libc/mem.h"
#include "../libc/string.h"

#define TASK_MAX 2

static Task tasks[TASK_MAX];

static void otherMain() {
    rainbow_print("Hello multitasking world!\n");
    yield();
    rainbow_print("Hello again!\n");
    yield();
}

void initTasking() {
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

    createTask(&otherTask, otherMain, mainTask.regs.eflags, (uint32_t*)mainTask.regs.cr3, 1);

    tasks[0] = mainTask;
    tasks[1] = otherTask;

    kprint("Tasking initialized\n");
}

void createTask(Task *task, void (*main)(), uint32_t flags, uint32_t *pagedir, int pid) {
    task->regs.eax = 0;
    task->regs.ebx = 0;
    task->regs.ecx = 0;
    task->regs.edx = 0;
    task->regs.esi = 0;
    task->regs.edi = 0;
    task->regs.eflags = flags;
    task->regs.eip = (uint32_t) main;
    task->regs.cr3 = (uint32_t) pagedir;
    task->regs.esp = (uint32_t) alloc_page(0);
    task->pid = pid;
    char str[10];
    kprint("Task created ");
    int_to_ascii(task->pid, str);
    ckprint(str, c_green);
    kprint("\n");
}

void print_switching(int stat, Task *task1, Task *task2) {
    char str[10];
    if (stat == 0) { kprint("before crossing: ");}
    else if (stat == 1) { kprint("after crossing: ");}
    else if (stat == 2) { kprint("after switching: ");}
    int_to_ascii(task1->pid, str);
    ckprint(str, c_green);
    kprint(" - ");
    int_to_ascii(task2->pid, str);
    ckprint(str, c_green);
    kprint("\n");
}

void yield() {
    print_switching(0, &tasks[0], &tasks[1]);

    for (int i = TASK_MAX; i > 0; i--) {
        tasks[i] = tasks[i - 1];
    }
    tasks[0] = tasks[TASK_MAX];

    print_switching(1, &tasks[0], &tasks[1]);

    switchTask(&tasks[1].regs, &tasks[0].regs);

    print_switching(2, &tasks[0], &tasks[1]);
}
