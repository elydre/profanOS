#define PROFAN_C

#include <syscall.h>
#include <filesys.h>
#include <profan.h>
#include <stdarg.h>
#include <libmmq.h>
#include <type.h>

// input() setings
#define FIRST_L 12
#define SLEEP_T 15
#define INP_CLR "\e[94m"
#define INP_RST "\e[0m"

#define ELF_INTERP "/bin/sys/deluge.bin"

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
        fd_printf(2, "Failed to load keyboard map\n");
        return 1;
    }

    return 0;
}

int userspace_reporter(char *message) {
    int len = 0;
    while (message[len] != '\0')
        len++;
    fm_write(2, message, len);
    return 0;
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

char *open_input_keyboard(int *size, char *term_path) {
    int fd = fm_open(term_path);
    if (fd == -1) {
        return NULL;
    }

    fd_putstr(fd, "\e[?25l");

    uint32_t buffer_actual_size, buffer_index, buffer_size;
    int sc, last_sc, last_sc_sgt, key_ticks, shift;

    char *buffer = malloc(100);
    buffer[0] = '\0';
    buffer_size = 100;

    sc = last_sc = last_sc_sgt = key_ticks = shift = 0;
    buffer_actual_size = buffer_index = 0;

    while (sc != ENTER) {
        c_process_sleep(c_process_get_pid(), SLEEP_T);
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
            fd_putstr(fd, "\e[1D");
        }

        else if (sc == RIGHT) {
            if (buffer_index == buffer_actual_size) continue;
            buffer_index++;
            fd_putstr(fd, "\e[1C");
        }

        else if (sc == BACK) {
            if (!buffer_index) continue;
            buffer_index--;
            for (uint32_t i = buffer_index; i < buffer_actual_size; i++) {
                buffer[i] = buffer[i + 1];
            }
            buffer[buffer_actual_size--] = '\0';
            fd_putstr(fd, "\e[1D\e[s"INP_CLR);
            fd_putstr(fd, buffer + buffer_index);
            fd_putstr(fd, INP_RST" \e[u");
        }

        else if (sc == DEL) {
            if (buffer_index == buffer_actual_size) continue;
            for (uint32_t i = buffer_index; i < buffer_actual_size; i++) {
                buffer[i] = buffer[i + 1];
            }
            buffer[buffer_actual_size--] = '\0';
            fd_putstr(fd, "\e[s"INP_CLR);
            fd_putstr(fd, buffer + buffer_index);
            fd_putstr(fd, INP_RST" \e[u");
        }

        else if (sc == ESC) {
            fm_close(fd);
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
            fd_putstr(fd, INP_CLR);
            fd_putchar(fd, c);
            if (buffer_index < buffer_actual_size) {
                for (uint32_t i = buffer_actual_size; i > buffer_index; i--) {
                    buffer[i] = buffer[i - 1];
                }
                fd_putstr(fd, "\e[s");
                fd_putstr(fd, buffer + buffer_index + 1);
                fd_putstr(fd, INP_RST"\e[u");
            } else
                fd_putstr(fd, INP_RST);
            buffer[buffer_index++] = c;
            buffer[++buffer_actual_size] = '\0';
        }
    }

    buffer[buffer_actual_size++] = '\n';
    buffer[buffer_actual_size] = '\0';
    fd_putstr(fd, "\e[?25h\n");
    fm_close(fd);

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

int run_ifexist_full(runtime_args_t args, int *pid_ptr) {
    if (args.path == NULL) {
        fd_printf(2, "file is required to run\n");
        return -1;
    }

    // duplicate argv
    args.argc += 2;
    int size = sizeof(char *) * (args.argc + 1);
    char **nargv = malloc_ask(size);
    memset((void *) nargv, 0, size);

    nargv[0] = (char *) malloc_ask(strlen(ELF_INTERP) + 1);
    strcpy(nargv[0], ELF_INTERP);
    nargv[1] = (char *) malloc_ask(strlen(args.path) + 1);
    strcpy(nargv[1], args.path);

    for (int i = 2; i < args.argc; i++) {
        nargv[i] = (char *) malloc_ask(strlen(args.argv[i-2]) + 1);
        strcpy(nargv[i], args.argv[i-2]);
    }

    // duplicate envp
    size = 0;
    if (args.envp != NULL) {
        while (args.envp[size] != NULL)
            size++;
    }
    char **nenvp = malloc_ask((size + 1) * sizeof(char *));
    memset((void *) nenvp, 0, (size + 1) * sizeof(char *));

    for (int i = 0; i < size; i++) {
        nenvp[i] = malloc_ask(strlen(args.envp[i]) + 1);
        strcpy(nenvp[i], args.envp[i]);
    }

    sid_t sid = fu_path_to_sid(ROOT_SID, ELF_INTERP);
    int pid = c_process_create(c_binary_exec, 1, args.path, 5, sid, args.argc, nargv, nenvp);

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
