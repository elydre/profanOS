#include <libc/ramdisk.h>
#include <gui/gnrtx.h>
#include <function.h>
#include <system.h>
#include <string.h>
#include <iolib.h>
#include <mem.h>

#define BFR_SIZE 65

int shell_command(char command[]);

void start_kshell() {
    sys_warning("You are now in the kernel-level shell");
    fskprint("\n");
    char char_buffer[BFR_SIZE];
    while (1) {
        rainbow_print("kernel-shell> ");
        input(char_buffer, BFR_SIZE, 9);
        fskprint("\n");
        if (shell_command(char_buffer)) break;
        char_buffer[0] = '\0';
    }
    clear_screen();
}


void shell_so(char suffix[]) {
    char path[100] = "/bin/";
    str_cat(path, suffix);
    str_cat(path, ".bin");
    fskprint("path: %s\n", path);
    run_ifexist(path, 0, (char **)0);    
}

void shell_help() {
    char *help[] = {
        "ADDR   - show main address",
        "ALLOC  - allocate *0x1000",
        "EXIT   - quit the kshell",
        "GO     - go file as binary",
        "HELP   - show this help",
        "MEM    - show memory state",
        "REBOOT - reboot the system",
        "SO     - run file in /bin",
    };

    for (int i = 0; i < ARYLEN(help); i++)
        fskprint("%s\n", help[i]);
}

void rgb_test() {
    uint32_t color;
    int ligne = get_offset_row(get_cursor_offset()) + 1;
    char buffer[10];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            color = (1<<16) * (i*32 + 110) + (1<<8) * (j*4 + 128) + 1 * (i*16 - j*6 + 128);
            hex_to_ascii(color, buffer);
            kprint_rgb_at(buffer, i * 15 + 4, ligne + j, 0, color);
        }
    }
    kprint("\n\n");
}

void shell_addr() {
    fskprint("physic:  %x (%fMo)\n", mem_get_phys_size(), mem_get_phys_size() / 1024.0 / 1024.0);
    fskprint("ramdisk: %x (%fMo)\n", ramdisk_get_address(), ramdisk_get_size() / 2048.0);
    fskprint("b3 mm:   %x\n", mem_get_base_addr());
    fskprint("watfunc: %x\n", WATFUNC_ADDR);
    fskprint("rand sv: %x\n", RAND_SAVE);
}

int shell_command(char command[]) {
    char prefix[BFR_SIZE], suffix[BFR_SIZE];
    str_cpy(prefix, command);
    str_cpy(suffix, command);
    str_start_split(prefix, ' ');
    str_end_split(suffix, ' ');

    if      (str_cmp(prefix, "addr") == 0) shell_addr();
    else if (str_cmp(prefix, "alloc") == 0) mem_alloc(ascii_to_int(suffix) * 0x1000);
    else if (str_cmp(prefix, "exit") == 0) return 1;
    else if (str_cmp(prefix, "go") == 0) run_ifexist(suffix, 0, (char **)0);
    else if (str_cmp(prefix, "help") == 0) shell_help();
    else if (str_cmp(prefix, "mem") == 0) mem_print();
    else if (str_cmp(prefix, "reboot") == 0) sys_reboot();
    else if (str_cmp(prefix, "so") == 0) shell_so(suffix);
    else if (str_cmp(prefix, "rgb") == 0) rgb_test();
    else if (prefix[0] != '\0') fskprint("$Bnot found: $3%s\n", prefix);

    return 0;
}
