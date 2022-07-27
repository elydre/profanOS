#include "../drivers/screen.h"
#include "../libc/string.h"
#include "../drivers/ata/ata.h"
#include "kernel.h"
#include "shell.h"
#include "task.h"
#include <stdint.h>

void shell_omp(){
    kprint("profanOS-> ");
}
 
void doIt() {
    kprint("Switching to otherTask... \n");
    yield();
    kprint("Returned to mainTask!\n");
}

void disk_test() {
    uint32_t inbytes[128];
    uint32_t outbytes[128];
    char tmp[3];

    ckprint("writing to disk\n", c_magenta);
    
    for (int i = 0; i < 128; i++) {
        inbytes[i] = i;
    }

    write_sectors_ATA_PIO(0, inbytes);
    ckprint("writing done\nreading from disk\n", c_magenta);
    
    read_sectors_ATA_PIO(0, outbytes);

    for (int i = 0; i < 128; i++) {
        if (outbytes[i] < 8) {
            int_to_ascii(outbytes[i], tmp);
            ckprint(tmp, c_magenta);
            kprint("\n");
        }
    }
    ckprint("...\nreading done, it works!\n", c_magenta);
}

void shell_help(char suffix[]) {
    char *help[] = {
        "CLEAR   - clear the screen",
        "ECHO    - print the arguments",
        "END     - shutdown the system",
        "HELP    - show this help",
        "ISR     - test the interrupt handler",
        "TD      - test the disk",
        "VER     - display the version",
    };

    int table_size = sizeof(help) / sizeof(char *);

    if (strcmp(suffix, "help") == 0) {
        for (int i = 0; i < table_size; i++) {
            ckprint(help[i], c_magenta);
            kprint("\n");
        }

    } else {
        char tmp[100];
        for (int i = 0; i < table_size; i++) {
            strcpy(tmp, help[i]);
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

void shell_command(char *command) {
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

    else if (strcmp(prefix, "fwrite") == 0) {
        ckprint(suffix, c_magenta);
        uint32_t inbytes[128] = {0};
        for (int i = 0; i < strlen(suffix); i++){
            inbytes[i] = suffix[i];
        }
        write_sectors_ATA_PIO(0, inbytes);
    }

    else if (strcmp(prefix, "help") == 0) {
        shell_help(suffix);
    }

    else if (strcmp(prefix, "isr") == 0) {
        asm volatile("int $0x1");
    }

    else if (strcmp(prefix, "task") == 0) {
        doIt();
    }

    else if (strcmp(prefix, "td") == 0) {
        disk_test();
    }

    else if (strcmp(prefix, "ver") == 0) {
        ckprint("version ", c_magenta);
        ckprint(VERSION, c_magenta);
        kprint("\n");
    }

    else if (strcmp(prefix, "") != 0) {
        ckprint(prefix, c_red);
        ckprint(" is not a valid command.\n", c_dred);
    }

    if (strcmp(prefix, "") * strcmp(prefix, "clear") != 0) { kprint("\n"); }

    shell_omp();
}
