/*****************************************************************************\
|   === extra.c : 2024 ===                                                    |
|                                                                             |
|    Extra functions for libC                                      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#define _SYSCALL_CREATE_FUNCS

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int serial_debug(char *frm, ...) {
    va_list args;
    char *str;
    int len;

    va_start(args, frm);
    str = malloc(1024);

    len = vsprintf(str, frm, args);
    syscall_serial_write(SERIAL_PORT_A, str, len);

    free(str);
    va_end(args);

    return len;
}

void profan_print_memory(void *addr, uint32_t size) {
    for (uint32_t i = 0; i < size / 16 + (size % 16 != 0); i++) {
        printf("%08x: ", (uint32_t) addr + i * 16);

        for (int j = 0; j < 16; j++) {
            if (i * 16 + j < size)
                printf("%02x ", *((unsigned char *) addr + i * 16 + j));
            else
                printf("   ");
            if (j % 4 == 3)
                printf(" ");
        }

        for (int j = 0; j < 16; j++) {
            unsigned char c = *((unsigned char *) addr + i * 16 + j);
            if (i * 16 + j >= size)
                break;
            if (c >= 32 && c <= 126)
                printf("%c", c);
            else
                printf(".");
        }
        printf("\n");
    }
}

char *assemble_path(const char *old, const char *new) {
    char *result;
    int len;

    if (new[0] == '/') {
        return strdup(new);
    }

    result = malloc(strlen(old) + strlen(new) + 2);
    strcpy(result, old);

    if (result[strlen(result) - 1] != '/') {
        strcat(result, "/");
    }
    strcat(result, new);

    len = strlen(result) - 1;
    if (result[len] == '/' && len > 0) {
        result[len] = '\0';
    }

    return result;
}

int profan_wait_pid(uint32_t pid) {
    uint32_t current_pid = syscall_process_get_pid();

    if (pid == current_pid || !pid)
        return 0;

    while (syscall_process_get_state(pid) < 4)
        syscall_process_sleep(current_pid, 10);

    return syscall_process_get_info(pid, PROCESS_INFO_EXIT_CODE);
}

char *open_input(int *size) {
    char *term = getenv("TERM");
    if (!term)
        return NULL;
    if (strstr(term, "serial"))
        return open_input_serial(size, SERIAL_PORT_A);
    return open_input_keyboard(size, term);
}

// defined in deluge
void profan_cleanup(void) {
    puts("libc extra: profan_cleanup: should not be called");
    return;
}
