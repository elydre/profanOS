#include <kernel/process.h>
#include <gui/gnrtx.h>
#include <minilib.h>
#include <system.h>

char *angel1 =
"\0\0"
"    ___      \0"
"   d6966b.   \0"
"   )u`d99b   \0"
"   \\_ q6b'  \0"
"    _) \\_   \0"
"   /   / \\  \0"
" _(___/  |   \0"
"|        |   \0"
"|       /    \0"
" \\__.--'\\  \0"
"  |     |    \0"
"  |     |    \0"
"  |     |    \0"
"  |     |    \0"
"  |     |    \0"
"  |     |    \0"
"  |__   |    \0"
"   |/`-.|    \0";

char *angel2 =
",-\"\"-.      \0"
"`-..-'   /\\/\\\0"
"        / /  )\0"
"       / /   (\0"
"        /    (\0"
"       /     )\0"
"      /     / \0"
"           (  \0"
"        __/  \0"
"       '    \0";

int size_x, size_y;

void sod_print_at(int x, int y, char *str, char color) {
    int i = 0;
    while (str[i] != '\0') {
        kprint_char_at(x + i, y, str[i], color);
        i++;
    }
}

void sod_print_generic_info(int is_cpu_error) {
    char str[64];
    int tmp;

    if (is_cpu_error) {
        sod_print_at(4, 1, "profanOS received an interrupt", 0x0D);
        sod_print_at(6, 3, "the CPU raised an unexpected interrupt, your", 0x05);
        sod_print_at(6, 4, "system has been halted to prevent damage...", 0x05);

    } else {
        sod_print_at(4, 1, "profanOS kernel FATAL ERROR", 0x0D);
        sod_print_at(6, 3, "the kernel has encountered a fatal error, your", 0x05);
        sod_print_at(6, 4, "system has been halted to prevent damage...", 0x05);
    }
    sod_print_at(6, 6, "kernel", 0x05);
    sod_print_at(13, 6, sys_kinfo(), 0x0D);

    sod_print_at(6, 7, "during", 0x05);
    process_get_name(process_get_pid(), str);
    sod_print_at(13, 7, str, 0x0D);
    tmp = str_len(str);

    int2str(process_get_pid(), str);
    sod_print_at(14 + tmp, 7, "- pid", 0x05);
    sod_print_at(20 + tmp, 7, str, 0x0D);
}

void sod_print_file_info(char *file_name, int line) {
    char str[10];
    sod_print_at(6, 9, "in", 0x05);
    sod_print_at(9, 9, file_name, 0x0D);

    sod_print_at(10 + str_len(file_name), 9, "at line", 0x05);
    int2str(line, str);
    sod_print_at(18 + str_len(file_name), 9, str, 0x0D);
}

void sod_print_angel() {
    int x, index = 0;
    int x_offset = size_x - 21;
    for (int i = 0; i < 20; i++) {
        for (x = 0; angel1[index] != '\0'; x++, index++) {
            if (angel1[index] == ' ')
                continue;
            kprint_char_at(x + x_offset, 2 + i, angel1[index], 0x0D);
        }
        index++;
    }

    index = 0;
    for (int i = 0; i < 10; i++) {
        for (x = 0; angel2[index] != '\0'; x++, index++) {
            if (angel2[index] == ' ')
                continue;
            kprint_char_at(x + x_offset + 3, 2 + i, angel2[index], 0x05);
        }
        index++;
    }

    for (int i = 0; i < size_y; i++) {
        kprint_char_at(0, i, ':', 0x05);
    }
}

void sod_stop(void) {
    asm volatile("cli");
    asm volatile("hlt");
}

void sod_fatal(char *file_name, int line, char *msg) {
    size_x = gt_get_max_cols();
    size_y = gt_get_max_rows();

    clear_screen();
    sod_print_angel();
    sod_print_generic_info(0);

    sod_print_file_info(file_name, line);

    sod_print_at(6, 11, "-> ", 0xD0);
    sod_print_at(9, 11, msg, 0xD0);


    sod_stop();
}
