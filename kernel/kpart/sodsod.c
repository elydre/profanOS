/*****************************************************************************\
|   === sodsod.c : 2024 ===                                                   |
|                                                                             |
|    Kernel SODSOD (Angelic screen of death)                       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/process.h>
#include <gui/gnrtx.h>
#include <minilib.h>
#include <system.h>

/***************************************
 *                                    *
 *  SODSOD - Angelic screen of death  *
 *                                    *
***************************************/

struct stackframe {
    struct stackframe* ebp;
    uint32_t eip;
};

int size_x, size_y;

/***************************************
 *                                    *
 *   Interrupts messages and angel    *
 *                                    *
***************************************/

char *angel1 =
"\0\0"
"    ___    \0"
"   d6966b. \0"
"   )u`d99b \0"
"   \\_ q6b'\0"
"    _) \\_ \0"
"   /   / \\\0"
" _(___/  | \0"
"|        | \0"
"|       /  \0"
" \\__.--'\\\0"
"  |     |  \0"
"  |     |  \0"
"  |     |  \0"
"  |     |  \0"
"  |     |  \0"
"  |     |  \0"
"  |__   |  \0"
"   |/`-.|  \0";

char *angel2 =
",-\"\"-.\0"
"`-..-'   /\\/\\\0"
"        / /  )\0"
"       / /   (\0"
"        /    (\0"
"       /     )\0"
"      /     /\0"
"           (\0"
"        __/\0"
"       '\0";

/***************************************
 *                                    *
 *    SODSOD graphics manipulation    *
 *                                    *
***************************************/

static void sod_print_at(int x, int y, char *str, char color) {
    int i = 0;
    while (str[i] != '\0') {
        kprint_char_at(x + i, y, str[i], color);
        i++;
    }
}

static void sod_putaddr_at(int x, int y, uint32_t addr, char color) {
    char str[10];
    for (int i = 0; i < 8; i++) {
        str[i < 4 ? i : i + 1] = "0123456789ABCDEF"[(addr >> (28 - i * 4)) & 0xF];
    }
    str[4] = ' ';
    str[9] = '\0';
    sod_print_at(x, y, "0x", color);
    sod_print_at(x + 3, y, str, color);
}

static void sod_print_generic_info(int is_cpu_error) {
    char str[64];
    int tmp;

    if (is_cpu_error) {
        sod_print_at(4, 1, "profanOS received an interrupt", 0x0D);
        sod_print_at(6, 3, "the CPU raised an unexpected interrupt, your", 0x05);
    } else {
        sod_print_at(4, 1, "profanOS kernel FATAL ERROR", 0x0D);
        sod_print_at(6, 3, "the kernel has encountered a fatal error, your", 0x05);
    }
    sod_print_at(6, 4, "system has been halted to prevent damage...", 0x05);

    sod_print_at(6, 6, "kernel", 0x05);
    sod_print_at(13, 6, sys_kinfo(), 0x0D);

    sod_print_at(6, 7, "during", 0x05);
    str_cpy(str, (char *) process_info(process_get_pid(), PROC_INFO_NAME, NULL));
    sod_print_at(13, 7, str, 0x0D);
    tmp = str_len(str);

    int2str(process_get_pid(), str);
    sod_print_at(14 + tmp, 7, "- pid", 0x05);
    sod_print_at(20 + tmp, 7, str, 0x0D);

    sod_print_at(4, size_y - 2, "and out of darkness - will guide you - the solitary angel", 0x05);
}

static void sod_print_file_info(char *file_name, int line) {
    char str[10];
    sod_print_at(6, 9, "in", 0x05);
    sod_print_at(9, 9, file_name, 0x0D);

    sod_print_at(10 + str_len(file_name), 9, "at line", 0x05);
    int2str(line, str);
    sod_print_at(18 + str_len(file_name), 9, str, 0x0D);
}

static void sod_print_angel() {
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

static void sod_print_stacktrace(void) {
    struct stackframe *stk;
    asm ("movl %%ebp,%0" : "=r"(stk) ::);
    if (stk == NULL) {
        sod_print_at(4, 13, "No stack trace available...", 0x05);
        return;
    }

    sod_print_at(4, 13, "Stack trace:", 0x05);
    for (int i = 14; stk->eip; i++) {
        if (i > size_y - 4) {
            sod_print_at(6, i, "[...]", 0x05);
            break;
        }
        sod_putaddr_at(6, i, stk->eip, 0x0D);
        if (stk->ebp == NULL) break;
        stk = stk->ebp;
    }
}

/***************************************
 *                                    *
 *       SODSOD main functions        *
 *                                    *
***************************************/

void sod_fatal(char *file_name, int line, char *msg, ...) {
    asm volatile("cli");
    va_list args;
    va_start(args, msg);
    kprintf_va2buf(sys_safe_buffer, msg, args);
    va_end(args);

    size_x = gt_get_max_cols();
    size_y = gt_get_max_rows();

    clear_screen();
    sod_print_angel();
    sod_print_generic_info(0);

    sod_print_file_info(file_name, line);

    sod_print_at(6, 11, "-> ", 0xD0);
    sod_print_at(9, 11, sys_safe_buffer, 0xD0);

    sod_print_stacktrace();

    asm volatile("hlt");
}

void sod_interrupt(uint8_t code, int err_code, char *msg) {
    asm volatile("cli");
    size_x = gt_get_max_cols();
    size_y = gt_get_max_rows();

    clear_screen();
    sod_print_angel();
    sod_print_generic_info(1);

    char str[20];
    int2str(code, str);
    str_cat(str, " (");
    int2str(err_code, str + str_len(str));
    str_cat(str, ")");

    sod_print_at(6, 9, "CPU raised interrupt", 0x05);
    sod_print_at(27, 9, str, 0x0D);

    sod_print_at(6, 11, "-> ", 0xD0);
    if (code < 19)
        sod_print_at(9, 11, msg, 0xD0);
    else
        sod_print_at(9, 11, "Unknown interrupt", 0xD0);

    sod_print_stacktrace();

    asm volatile("hlt");
}
