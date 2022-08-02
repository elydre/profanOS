#include "task.h"
#include "../drivers/screen.h"
#include "../libc/mem.h"
#include "../libc/string.h"

#define TASK_MAX 2

static Task tasks[TASK_MAX];

static void otherMain() {
    rainbow_print("Hello multitasking world!\n");
    yield("otherMain");
    rainbow_print("Hello again!\n");
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

    strcpy(mainTask.name, "main");

    createTask(&otherTask, otherMain, mainTask.regs.eflags, (uint32_t*)mainTask.regs.cr3, "other");

    tasks[0] = mainTask;
    tasks[1] = otherTask;
    kprint("Tasking initialized\n");
}

void createTask(Task *task, void (*main)(), uint32_t flags, uint32_t *pagedir, char name[]) {
    task->regs.eax = 0;
    task->regs.ebx = 0;
    task->regs.ecx = 0;
    task->regs.edx = 0;
    task->regs.esi = 0;
    task->regs.edi = 0;
    task->regs.eflags = flags;
    task->regs.eip = (uint32_t) main;
    task->regs.cr3 = (uint32_t) pagedir;
    task->regs.esp = (uint32_t) kmalloc(0x1000, 1, 0);
    strcpy(task->name, name);
    char str[10];
    int_to_ascii(task->regs.eip, str);
    kprint("Task created ");
    ckprint(name, c_green);
    kprint(" (");
    ckprint(str, c_cyan);
    kprint(")\n");
}

void print_switching(char func_name[], Task *task1, Task *task2) {
    kprint("Yielded from ");
    ckprint(func_name, c_green);
    char str[10];
    kprint(" - switching from ");
    ckprint(task1->name, c_green);
    kprint(" (");
    int_to_ascii(task1->regs.eip, str);
    ckprint(str, c_cyan);
    kprint(") to ");
    ckprint(task2->name, c_green);
    kprint(" (");
    int_to_ascii(task2->regs.eip, str);
    ckprint(str, c_cyan);
    kprint(")\n");
}

void copy_task(Task *task1, Task *task2) {
    task1->regs = task2->regs;
    strcpy(task2->name, task1->name);
}

void yield(char func_name[]) {
    print_switching(func_name, &tasks[0], &tasks[1]);

    copy_task(&tasks[0], &tasks[1]);
    copy_task(&tasks[2], &tasks[0]);
    copy_task(&tasks[1], &tasks[2]);

    print_switching(func_name, &tasks[0], &tasks[1]);

    switchTask(&tasks[1].regs, &tasks[0].regs);

    print_switching(func_name, &tasks[0], &tasks[1]);
}
