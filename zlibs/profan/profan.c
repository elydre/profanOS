#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>

#define PROFAN_C

#include <syscall.h>
#include <filesys.h>
#include <profan.h>
#include <type.h>

// input() setings
#define FIRST_L 12
#define SLEEP_T 15
#define INP_CLR "\e[94m"
#define INP_RST "\e[0m"

// keyboard scancodes
#define ESC     1
#define BACK    14
#define ENTER   28
#define LSHIFT  42
#define RSHIFT  54
#define SC_MAX  57
#define LEFT    75
#define RIGHT   77
#define DEL     83
#define RESEND  224

struct stackframe {
    struct stackframe* ebp;
    uint32_t eip;
};

char *kb_map;

int profan_kb_load_map(char *path);

int main(void) {
    kb_map = NULL;
    if (profan_kb_load_map("/zada/keymap/azerty.map")) {
        printf("Failed to load keyboard map\n");
        return 1;
    }

    return 0;
}

int userspace_reporter(char *message) {
    char *term = getenv("TERM");
    int c = fputs(message, stderr);

    if (term && strcmp(term, "/dev/serial")) {
        c_serial_write(SERIAL_PORT_A, message, strlen(message));
    }
    return c == EOF;
}

char *assemble_path(char *old, char *new) {
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

void profan_print_stacktrace(void) {
    struct stackframe *stk;
    asm ("movl %%ebp,%0" : "=r"(stk) ::);
    printf("Stack trace:\n");
    int size = 0;
    while (stk->eip) {
        printf("   %x\n", stk->eip);
        stk = stk->ebp;
        size++;
    }
    printf("total size: %d\n", size);
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

int profan_kb_load_map(char *path) {
    sid_t sid = fu_path_to_sid(ROOT_SID, path);
    if (IS_NULL_SID(sid)) {
        return 1;
    }

    int file_size = fu_get_file_size(sid);
    char *file_content = malloc(file_size + 1);

    fu_file_read(sid, file_content, 0, file_size);
    file_content[file_size] = '\0';

    if (strncmp(file_content, "#KEYMAP", 7)) {
        free(file_content);
        return 1;
    }

    char *tmp = calloc_ask(128, 1);

    int tmp_i = 0;
    for (int i = 7; i < file_size; i++) {
        if (file_content[i] < 32 || file_content[i] > 126)
            continue;
        if (file_content[i] == '\\') {
            i++;
            if (file_content[i] == '\\') {
                tmp[tmp_i++] = '\\';
            } else if (file_content[i] == '0') {
                tmp[tmp_i++] = '\0';
            } else {
                tmp[tmp_i++] = file_content[i];
            }
        } else {
            tmp[tmp_i++] = file_content[i];
        }
        if (tmp_i == 128) {
            break;
        }
    }

    free(file_content);
    free(kb_map);
    kb_map = tmp;
    return 0;
}

char profan_kb_get_char(uint8_t scancode, uint8_t shift) {
    if (scancode > 64)
        return '\0';
    if (kb_map == NULL)
        return c_kb_scancode_to_char(scancode, shift);
    if (shift)
        return kb_map[scancode * 2 + 1];
    return kb_map[scancode * 2];
}

int serial_debug(char *frm, ...);

int profan_wait_pid(uint32_t pid) {
    uint32_t current_pid = c_process_get_pid();

    if (pid == current_pid || !pid)
        return 0;

    while (c_process_get_state(pid) < 4)
        c_process_sleep(current_pid, 10);

    return c_process_get_info(pid, PROCESS_INFO_EXIT_CODE);
}

char *open_input_keyboard(int *size, char *term_path) {
    FILE *term = fopen(term_path, "w");

    fputs("\e[?25l", term);
    fflush(term);

    uint32_t buffer_actual_size, buffer_index, buffer_size;
    int sc, last_sc, last_sc_sgt, key_ticks, shift;

    char *buffer = malloc(100);
    buffer[0] = '\0';
    buffer_size = 100;

    sc = last_sc = last_sc_sgt = key_ticks = shift = 0;
    buffer_actual_size = buffer_index = 0;

    while (sc != ENTER) {
        usleep(SLEEP_T * 1000);
        sc = c_kb_get_scfh();

        if (sc == RESEND || sc == 0) {
            sc = last_sc_sgt;
        } else {
            last_sc_sgt = sc;
        }

        key_ticks = (sc != last_sc) ? 0 : key_ticks + 1;
        last_sc = sc;

        if ((key_ticks < FIRST_L && key_ticks) || key_ticks % 2) {
            continue;
        }

        if (sc == LSHIFT || sc == RSHIFT) {
            shift = 1;
            continue;
        }

        if (sc == LSHIFT + 128 || sc == RSHIFT + 128) {
            shift = 0;
            continue;
        }

        if (sc == LEFT) {
            if (!buffer_index) continue;
            buffer_index--;
            fputs("\e[1D", term);
        }

        else if (sc == RIGHT) {
            if (buffer_index == buffer_actual_size) continue;
            buffer_index++;
            fputs("\e[1C", term);
        }

        else if (sc == BACK) {
            if (!buffer_index) continue;
            buffer_index--;
            for (uint32_t i = buffer_index; i < buffer_actual_size; i++) {
                buffer[i] = buffer[i + 1];
            }
            buffer[buffer_actual_size--] = '\0';
            fputs("\e[1D\e[s"INP_CLR, term);
            fputs(buffer + buffer_index, term);
            fputs(INP_RST" \e[u", term);
        }

        else if (sc == DEL) {
            if (buffer_index == buffer_actual_size) continue;
            for (uint32_t i = buffer_index; i < buffer_actual_size; i++) {
                buffer[i] = buffer[i + 1];
            }
            buffer[buffer_actual_size--] = '\0';
            fputs("\e[s"INP_CLR, term);
            fputs(buffer + buffer_index, term);
            fputs(INP_RST" \e[u", term);
        }

        else if (sc == ESC) {
            fclose(term);
            buffer = realloc(buffer, buffer_actual_size + 1);
            if (size)
                *size = buffer_actual_size;
            return buffer;
        }

        else if (sc <= SC_MAX) {
            char c = profan_kb_get_char(sc, shift);
            if (c == '\0') continue;
            if (buffer_size < buffer_actual_size + 2) {
                buffer_size *= 2;
                buffer = realloc(buffer, buffer_size);
            }
            fputs(INP_CLR, term);
            fputc(c, term);
            if (buffer_index < buffer_actual_size) {
                for (uint32_t i = buffer_actual_size; i > buffer_index; i--) {
                    buffer[i] = buffer[i - 1];
                }
                fputs("\e[s", term);
                fputs(buffer + buffer_index + 1, term);
                fputs(INP_RST"\e[u", term);
            } else
                fputs(INP_RST, term);
            buffer[buffer_index++] = c;
            buffer[++buffer_actual_size] = '\0';
        }

        else continue;
        fflush(term);
    }

    buffer[buffer_actual_size++] = '\n';
    buffer[buffer_actual_size] = '\0';
    fputs("\e[?25h\n", term);
    fclose(term);

    buffer = realloc(buffer, buffer_actual_size + 1);
    if (size)
        *size = buffer_actual_size;
    return buffer;
}

char *open_input_serial(int *size, int serial_port) {
    char *buffer = malloc(100);
    int buffer_size = 100;
    int i = 0;
    char c = 0;

     while (c != '\n') {
        c_serial_read(serial_port, &c, 1);
        if (c == '\r') {
            c_serial_write(serial_port, "\r", 1);
            c = '\n';
        }
        if (c == 127) {
            if (i) {
                i--;
                c_serial_write(serial_port, "\b \b", 3);
            }
            continue;
        }
        if ((c < 32 || c > 126) && c != '\n')
            continue;
        ((char *) buffer)[i++] = c;
        c_serial_write(serial_port, &c, 1);
        if (i == buffer_size) {
            buffer_size *= 2;
            buffer = realloc(buffer, buffer_size);
        }
    }

    buffer = realloc(buffer, i + 1);
    buffer[i] = '\0';

    if (size)
        *size = i;
    return buffer;
}

char *open_input(int *size) {
    char *term = getenv("TERM");
    if (!term)
        return NULL;
    if (strstr(term, "serial"))
        return open_input_serial(size, SERIAL_PORT_A);
    return open_input_keyboard(size, term);
}

int serial_debug(char *frm, ...) {
    va_list args;
    char *str;
    int len;

    va_start(args, frm);
    str = malloc(1024);

    len = vsprintf(str, frm, args);
    c_serial_write(SERIAL_PORT_A, str, len);

    free(str);
    va_end(args);

    return len;
}

int profan_open(char *path, int flags, ...) {
    // mode is ignored, permissions are always 777

    char *cwd = getenv("PWD");
    char *fullpath = cwd ? assemble_path(cwd, path) : strdup(path);

    sid_t sid = fu_path_to_sid(ROOT_SID, fullpath);
    if (IS_NULL_SID(sid) && (flags & O_CREAT)) {
        sid = fu_file_create(0, fullpath);
    }

    if (IS_NULL_SID(sid)) {
        free(fullpath);
        return -1;
    }

    if (flags & O_TRUNC && fu_is_file(sid)) {
        fu_set_file_size(sid, 0);
    }

    int fd = fm_open(fullpath);

    if (fd < 0) {
        free(fullpath);
        return -1;
    }

    if (flags & O_APPEND) {
        fm_lseek(fd, 0, SEEK_END);
    }

    free(fullpath);
    return fd;
}

int run_ifexist_full(runtime_args_t args, int *pid_ptr) {
    if (args.path == NULL && IS_NULL_SID(args.sid)) {
        return -1;
    }

    args.sid = IS_NULL_SID(args.sid) ? fu_path_to_sid(ROOT_SID, args.path) : args.sid;
    if (IS_NULL_SID(args.sid) || !fu_is_file(args.sid)) {
        return -1;
    }

    // duplicate argv
    int size = sizeof(char *) * (args.argc + 1);
    char **nargv = (char **) c_mem_alloc(size, 0, 6);
    memset((void *) nargv, 0, size);

    for (int i = 0; i < args.argc; i++) {
        nargv[i] = (char *) c_mem_alloc(strlen(args.argv[i]) + 1, 0, 6);
        strcpy(nargv[i], args.argv[i]);
    }

    int pid = c_process_create(c_binary_exec, 1, args.path ? args.path : "unknown file", 4, args.sid, args.vcunt, nargv);

    if (pid_ptr != NULL)
        *pid_ptr = pid;
    if (pid == -1)
        return -1;

    if (args.sleep_mode == 2)
        return 0;

    if (args.sleep_mode)
        c_process_handover(pid);
    else
        c_process_wakeup(pid);

    return c_process_get_info(pid, PROCESS_INFO_EXIT_CODE);
}
