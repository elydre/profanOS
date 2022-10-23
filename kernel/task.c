#include <driver/keyboard.h>
#include <driver/screen.h>
#include <driver/serial.h>
#include <gui/vgui.h>
#include <gui/vga.h>
#include <string.h>
#include <iolib.h>
#include <task.h>
#include <mem.h>

#define TASK_MAX 10

static task_t tasks[TASK_MAX + 1];
int current_pid, task_count;

/***********************
 * INTERNAL FUNCTIONS *
***********************/

void i_new_task(task_t *task, void (*main)(), uint32_t flags, uint32_t *pagedir, int pid) {
    uint32_t esp_alloc = (uint32_t) (uint32_t) mem_alloc(0x1000);
    task->regs.eax = 0;
    task->regs.ebx = 0;
    task->regs.ecx = 0;
    task->regs.edx = 0;
    task->regs.esi = 0;
    task->regs.edi = 0;
    task->regs.eflags = flags;
    task->regs.eip = (uint32_t) main;
    task->regs.cr3 = (uint32_t) pagedir;
    task->regs.esp = esp_alloc;
    task->esp_addr = esp_alloc;
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
        mem_free_addr(tasks[i].esp_addr);
        tasks[i].isdead = 2;
        fskprint("$Etask %d killed\n", tasks[i].pid);
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

    str_cpy(mainTask.name, "kernel");
    mainTask.vgui_save = 0;
    mainTask.gui_mode = 0;
    mainTask.isdead = 0;
    mainTask.pid = 0;

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
    task.vgui_save = 0;
    str_cpy(task.name, name);

    i_new_task(&task, func, mainTask->regs.eflags, (uint32_t*) mainTask->regs.cr3, pid);
    tasks[nb_alive] = task;
    task_count++;
    return pid;
}

void yield(int target_pid) {
    int task_i, nb_alive = i_refresh_alive();

    if (tasks[0].pid == target_pid) {
        sys_error("Cannot yield to self");
        return;
    }

    if (tasks[0].gui_mode && vgui_get_refresh_mode()) {
        serial_debug("YIELD", "save vgui");
        tasks[0].vgui_save = (vgui_get_refresh_mode() == 3) ? 2 : 1;
        vgui_exit();
    }

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
    if (tasks[0].vgui_save && tasks[0].gui_mode) {
        vgui_setup(tasks[0].vgui_save - 1);
    }

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
    sys_error("Task not found in kill");
}

void task_update_gui_mode(int mode) {
    tasks[0].gui_mode = mode;
    serial_debug("TASK", "update gui mode");
}

/******************
 * GET FUNCTIONS *
******************/

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

void task_set_bin_mem(int pid, char * bin_mem) {
    int nb_alive = i_refresh_alive();
    for (int i = 0; i < nb_alive; i++) {
        if (tasks[i].pid == pid) {
            tasks[i].bin_mem = bin_mem;
            return;
        }
    }
    sys_error("Task not found");
}
char * task_get_bin_mem(int pid) {
    int nb_alive = i_refresh_alive();
    for (int i = 0; i < nb_alive; i++) {
        if (tasks[i].pid == pid) {
            return tasks[i].bin_mem;
        }
    }
    sys_error("Task not found");
    return 0;
}

void task_debug_print() {
    int nb_alive = i_refresh_alive();
    for (int i = 0; i < nb_alive; i++) {
        fskprint("%sTask %s: %d (%d, %d, %x)\n",
            (i == 0) ? "$1" : "$4",
            tasks[i].name,
            tasks[i].pid,
            tasks[i].isdead,
            tasks[i].gui_mode,
            tasks[i].bin_mem
        );
    }
}

/****************
 * SWITCH MENU *
****************/

#define KB_KEY_UP 0x48
#define KB_KEY_DOWN 0x50
#define KB_KEY_ENTER 0x1C

void task_switch_menu() {
    // simple menu to switch between tasks with the keyboard arrows
    int selected_task = 0, nb_alive, key;
    while (1) {
        // refresh tasks
        nb_alive = i_refresh_alive();
        // print tasks
        for (int i = 0; i < nb_alive; i++) {
            ckprint_at(tasks[i].name, 0, i, (i == selected_task) ? 0xB0 : 0x0B);
        }
        // wait for key
        key = kb_get_scancode();
        if (key == KB_KEY_UP) {
            selected_task--;
            if (selected_task < 0) {
                selected_task = nb_alive - 1;
            }
        } else if (key == KB_KEY_DOWN) {
            selected_task++;
            if (selected_task >= nb_alive) {
                selected_task = 0;
            }
        } else if (key == KB_KEY_ENTER) {
            clear_screen();
            yield(tasks[selected_task].pid);
        }
        while (kb_get_scancode() == key);
    }
}
