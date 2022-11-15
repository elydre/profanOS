#include <libc/ramdisk.h>
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

void shell_addr() {
    fskprint("physic:  %x (%fMo)\n", mem_get_phys_size(), mem_get_phys_size() / 1024.0 / 1024.0);
    fskprint("ramdisk: %x (%fMo)\n", ramdisk_get_address(), ramdisk_get_size() / 2048.0);
    fskprint("b3 mm:   %x\n", mem_get_base_addr());
    fskprint("watfunc: %x\n", WATFUNC_ADDR);
    fskprint("rand sv: %x\n", RAND_SAVE);
}

void rgb_square() {
    // set pixel color in a 255x255 square
    uint32_t color; // 0x00RRGGBB
    while (1) {
        for (int k = 255; k > 0; k=k-7) {
            for (int i = 0; i < 255; i++) {
                for (int j = 0; j < 255; j++) {
                    color = (k << 16) + (i << 8) + j;
                    // we color in a square
                    for (int x = 0; x < 3 ; x++) {
                        for (int y = 0; y < 3; y++) {
                            vesa_set_pixel(i*3+x, j*3+y, color);
                        }
                    }
                }
            }
        }
        for (int k = 0; k < 255; k=k+7) {
            for (int i = 0; i < 255; i++) {
                for (int j = 0; j < 255; j++) {
                    color = (k << 16) + (i << 8) + j;
                    // we color in a square
                    for (int x = 0; x < 3 ; x++) {
                        for (int y = 0; y < 3; y++) {
                            vesa_set_pixel(i*3+x, j*3+y, color);
                        }
                    }
                }
            }
        }
    }
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
    else if (str_cmp(prefix, "rgb") == 0) rgb_square();
    else if (str_cmp(prefix, "so") == 0) shell_so(suffix);
    else if (prefix[0] != '\0') fskprint("$Bnot found: $3%s\n", prefix);

    return 0;
}
