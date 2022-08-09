#include "../drivers/screen.h"
#include "../libc/string.h"
#include "../drivers/ata/ata.h"
#include "../libc/function.h"
#include "../libc/mem.h"
#include "kernel.h"
#include "shell.h"
#include "task.h"
#include <stdint.h>

void shell_omp(){
    kprint("profanOS-> ");
}

void disk_test() {
    uint32_t inbytes[128];
    uint32_t outbytes[128];
    int sum = 0;
    char tmp[3];

    ckprint("writing to disk\n", c_magenta);
    
    for (int i = 0; i < 128; i++) {
        inbytes[i] = i;
    }

    write_sectors_ATA_PIO(0, inbytes);
    ckprint("writing done\nreading from disk\n", c_magenta);
    
    read_sectors_ATA_PIO(0, outbytes);

    for (int i = 0; i < 8; i++) {
        sum += outbytes[i];
        int_to_ascii(outbytes[i], tmp);
        ckprint(tmp, c_magenta);
        kprint("\n");
    }
    if (sum == 28) {
        ckprint("...\nsum == 28, it works!\n", c_magenta);
    } else {
        ckprint("...\nsum != 28, it doesn't work!\n", c_red);
    }
}

void show_disk_LBA(char suffix[]) {
    int LBA = 0;
    if (strcmp(suffix, "ss") != 0) {
        LBA = ascii_to_int(suffix);
    }
    uint32_t outbytes[128];
    char tmp[10];
    read_sectors_ATA_PIO((uint32_t) LBA, outbytes);
    for (int i = 0; i < 128; i++) {
        int_to_ascii(outbytes[i], tmp);
        ckprint(tmp, c_magenta);
        for (int j = strlen(tmp); j < 10; j++) {
            kprint(" ");
        }
    }
}

void page_info(){
    uint32_t page = alloc_page(1);
    ckprint("actual page address: ", c_magenta);
    char tmp[11];
    hex_to_ascii(page, tmp);
    ckprint(tmp, c_magenta);
    kprint("\n");
}

void shell_help(char suffix[]) {
    char *help[] = {
        "CLEAR   - clear the screen",
        "ECHO    - print the arguments",
        "END     - shutdown the system",
        "HELP    - show this help",
        "ISR     - test the interrupt handler",
        "PAGE    - show the actual page address",
        "SS      - show int32 in the LBA *suffix*",
        "TASK    - print info about the tasks",
        "TD      - test the disk",
        "VER     - display the version",
        "YIELD   - yield to the next task",
    };

    if (strcmp(suffix, "help") == 0) {
        for (int i = 0; i < ARYLEN(help); i++) {
            ckprint(help[i], c_magenta);
            kprint("\n");
        }

    } else {
        char tmp[100];
        for (int i = 0; i < ARYLEN(help); i++) {
            strcpy_s(tmp, help[i]);
            str_start_split(tmp, ' ');

            if (strcmp(tmp, suffix) == 0) {
                ckprint(help[i], c_magenta);
                kprint("\n");
                return;
            }
        }
        ckprint("command not found\n", c_red);
    }
}

void shell_command(char command[]) {
    char prefix[strlen(command)], suffix[strlen(command)];
    strcpy(prefix, command);
    strcpy(suffix, command);
    str_start_split(prefix, ' ');
    str_end_split(suffix, ' ');
    
    if (strcmp(prefix, "clear") == 0) {
        clear_screen();
    }

    else if (strcmp(prefix, "echo") == 0) {
        ckprint(suffix, c_magenta);
        kprint("\n");
    }

    else if (strcmp(prefix, "end") == 0) {
        rainbow_print("Stopping the CPU. Bye!\n");
        asm volatile("hlt");
    }

    else if (strcmp(prefix, "help") == 0) {
        shell_help(suffix);
    }

    else if (strcmp(prefix, "isr") == 0) {
        asm volatile("int $0x1");
    }

    else if (strcmp(prefix, "page") == 0) {
        page_info();
    }

    else if (strcmp(prefix, "ss") == 0) {
        show_disk_LBA(suffix);
    }

    else if (strcmp(prefix, "task") == 0) {
        task_printer();
    }

    else if (strcmp(prefix, "td") == 0) {
        disk_test();
    }

    else if (strcmp(prefix, "ver") == 0) {
        ckprint("version ", c_magenta);
        ckprint(VERSION, c_magenta);
        kprint("\n");
    }

    else if (strcmp(prefix, "yield") == 0) {
        yield();
    }

    else if (strcmp(prefix, "") != 0) {
        ckprint(prefix, c_red);
        ckprint(" is not a valid command.\n", c_dred);
    }

    if (strcmp(prefix, "") * strcmp(prefix, "clear") != 0) { kprint("\n"); }

    shell_omp();
}
