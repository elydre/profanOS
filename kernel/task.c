#include "task.h"
#include "../drivers/screen.h"
#include "../libc/mem.h"
#include "../libc/string.h"
 
static Task *runningTask;
static Task mainTask;
static Task otherTask;
 
static void otherMain() {
    ckprint("Hello multitasking world!\n", c_yellow);
    yield();
}

void initTasking() {
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
 
    createTask(&otherTask, otherMain, mainTask.regs.eflags, (uint32_t*)mainTask.regs.cr3);
    mainTask.next = &otherTask;
    otherTask.next = &mainTask;
 
    runningTask = &mainTask;
    kprint("Tasking initialized\n");
}
 
void createTask(Task *task, void (*main)(), uint32_t flags, uint32_t *pagedir) {
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
    task->next = 0;
    char str[10];
    int_to_ascii(task->regs.eip, str);
    kprint("Task created at ");
    ckprint(str, c_green);
    kprint("\n");
}

void yield() {
    Task *last = runningTask;
    runningTask = runningTask->next;

    char str1[10];
    char str2[10];
    int_to_ascii(last->regs.eip, str1);
    int_to_ascii(runningTask->regs.eip, str2);
    kprint("Switching from ");
    ckprint(str1, c_green);
    kprint(" to ");
    ckprint(str2, c_green);
    kprint("\n");

    switchTask(&last->regs, &runningTask->regs);
    initTasking();
}
