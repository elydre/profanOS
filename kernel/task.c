#include "task.h"
#include "../drivers/screen.h"
#include "../libc/mem.h"
#include "../libc/string.h"

#define TASK_MAX 3

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

    mainTask.pid = 0;

    createTask(&otherTask, otherMain, mainTask.regs.eflags, (uint32_t*)mainTask.regs.cr3, 1);

    tasks[0] = mainTask;
    tasks[1] = otherTask;
    tasks[2] = otherTask;
    tasks[3] = otherTask;

    tasks[2].pid = 42;

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
    task->regs.esp = (uint32_t) kmalloc(0x1000, 1, 0);
    task->pid = pid;
    char str[10];
    kprint("Task created ");
    int_to_ascii(task->pid, str);
    ckprint(str, c_green);
    kprint(" (");
    int_to_ascii(task->regs.eip, str);
    ckprint(str, c_cyan);
    kprint(")\n");
}

void print_switching(char func_name[], int stat, Task *task1, Task *task2) {
    kprint("Yielded from ");
    ckprint(func_name, c_green);
    char str[10];
    if (stat == 0) { kprint(" before switching: ");}
    else if (stat == 1) { kprint(" after switching: ");}
    else if (stat == 2) { kprint(" after asm: ");}
    else { kprint(" : ");}
    int_to_ascii(task1->pid, str);
    ckprint(str, c_green);
    kprint(" (");
    int_to_ascii(task1->regs.eip, str);
    ckprint(str, c_cyan);
    kprint(") - ");
    int_to_ascii(task2->pid, str);
    ckprint(str, c_green);
    kprint(" (");
    int_to_ascii(task2->regs.eip, str);
    ckprint(str, c_cyan);
    kprint(")\n");
}

void yield(char func_name[]) {
    print_switching(func_name, 0, &tasks[0], &tasks[1]);

    if (!(tasks[2].pid == 42) && tasks[1].pid == 1) {
        rainbow_print("patching\n");
        tasks[1] = tasks[3];
    }

    // tasks[]               t1 t2 .
    tasks[2] = tasks[1];  // t1 t2 t2
    tasks[1] = tasks[0];  // t1 t1 t2
    tasks[0] = tasks[2];  // t2 t1 t2

    print_switching(func_name, 1, &tasks[0], &tasks[1]);

    switchTask(&tasks[1].regs, &tasks[0].regs);

    print_switching(func_name, 2, &tasks[0], &tasks[1]);

    print_switching(func_name, 3, &tasks[0], &tasks[1]);
}
