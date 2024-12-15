/*****************************************************************************\
|   === profan.c : 2024 ===                                                   |
|                                                                             |
|    Usefull functions for profanOS (wiki/lib_profan)              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#define PROFAN_C

#define _SYSCALL_CREATE_STATIC
#include <profan/syscall.h>

#include <profan/filesys.h>
#include <profan/libmmq.h>
#include <profan.h>

#include <stdarg.h>
#include <fcntl.h> // for flags

#define DEFAULT_KB "/zada/keymap/azerty.map"
#define ELF_INTERP "/bin/sys/deluge.elf"
#define ENV_INTERP "DEFRUN"

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
    if (profan_kb_load_map(DEFAULT_KB)) {
        fd_printf(2, "[profan module] failed to load default keymap\n");
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
    uint32_t sid = fu_path_to_sid(SID_ROOT, path);
    if (IS_SID_NULL(sid)) {
        return 1;
    }

    int file_size = fu_file_get_size(sid);
    char *file_content = kmalloc(file_size + 1);

    fu_file_read(sid, file_content, 0, file_size);
    file_content[file_size] = '\0';

    if (str_ncmp(file_content, "#KEYMAP", 7)) {
        kfree(file_content);
        return 1;
    }

    char *tmp = kcalloc_ask(128, 1);

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

    kfree(file_content);
    kfree(kb_map);
    kb_map = tmp;
    return 0;
}

char profan_kb_get_char(uint8_t scancode, uint8_t shift) {
    if (scancode > 64)
        return '\0';
    if (kb_map == NULL)
        return syscall_sc_to_char(scancode, shift);
    if (shift)
        return kb_map[scancode * 2 + 1];
    return kb_map[scancode * 2];
}

char *profan_input_keyboard(int *size, char *term_path) {
    int fd = fm_open(term_path, O_RDWR);
    if (fd < 0) {
        return NULL;
    }

    fd_putstr(fd, "\e[?25l");

    uint32_t buffer_actual_size, buffer_index, buffer_size;
    int sc, last_sc, last_sc_sgt, key_ticks, shift;

    char *buffer = kmalloc(100);
    buffer[0] = '\0';
    buffer_size = 100;

    sc = last_sc = last_sc_sgt = key_ticks = shift = 0;
    buffer_actual_size = buffer_index = 0;

    while (sc != ENTER) {
        syscall_process_sleep(syscall_process_pid(), SLEEP_T);
        sc = syscall_sc_get();

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
            buffer = krealloc(buffer, buffer_actual_size + 1);
            if (size)
                *size = buffer_actual_size;
            return buffer;
        }

        else if (sc <= SC_MAX) {
            char c = profan_kb_get_char(sc, shift);
            if (c == '\0') continue;
            if (buffer_size < buffer_actual_size + 2) {
                buffer_size *= 2;
                buffer = krealloc(buffer, buffer_size);
            }
            fd_putstr(fd, INP_CLR);
            fd_putchar(fd, c);
            if (buffer_index < buffer_actual_size) {
                for (uint32_t i = buffer_actual_size + 1; i > buffer_index; i--) {
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

    buffer = krealloc(buffer, buffer_actual_size + 1);
    if (size)
        *size = buffer_actual_size;
    return buffer;
}

char *profan_input_serial(int *size, int serial_port) {
    char *buffer = kmalloc(100);
    int buffer_size = 100;
    int i = 0;
    char c = 0;

     while (c != '\n') {
        syscall_serial_read(serial_port, &c, 1);
        if (c == '\r') {
            syscall_serial_write(serial_port, "\r", 1);
            c = '\n';
        }
        if (c == 127) {
            if (i) {
                i--;
                syscall_serial_write(serial_port, "\b \b", 3);
            }
            continue;
        }
        if ((c < 32 || c > 126) && c != '\n')
            continue;
        ((char *) buffer)[i++] = c;
        syscall_serial_write(serial_port, &c, 1);
        if (i == buffer_size) {
            buffer_size *= 2;
            buffer = krealloc(buffer, buffer_size);
        }
    }

    buffer = krealloc(buffer, i + 1);
    buffer[i] = '\0';

    if (size)
        *size = i;
    return buffer;
}

static char **dup_envp(char **envp) {
    if (envp == NULL)
        return kcalloc_ask(1, sizeof(char *));
    int envc, size = 0;

    for (envc = 0; envp[envc] != NULL; envc++)
        size += str_len(envp[envc]) + 1;
    size += (envc + 1) * sizeof(char *);

    char **nenvp = kcalloc_ask(1, size);

    char *nenvp_start = (char *) nenvp + (envc + 1) * sizeof(char *);

    for (int i = 0; envp[i] != NULL; i++) {
        for (envc = 0; envp[i][envc] != '\0'; envc++) {
            nenvp_start[envc] = envp[i][envc];
        }
        nenvp[i] = nenvp_start;
        nenvp_start += envc + 1;
    }

    return nenvp;
}

static char **split_interp(char *tmp, int *c) {
    char **interp = kcalloc(1, sizeof(char *));
    *c = 0;

    for (int from, i = 0; tmp[i];) {
        while (tmp[i] == ' ' || tmp[i] == '\t' || tmp[i] == '\r')
            i++;
        from = i;
        while (tmp[i] && tmp[i] != ' ' && tmp[i] != '\t' && tmp[i] != '\r')
            i++;
        if (i == from)
            break;
        interp = krealloc(interp, (*c + 2) * sizeof(char *));
        char *cpy = kmalloc_ask(i - from + 1);
        mem_cpy(cpy, tmp + from, i - from);
        cpy[i - from] = '\0';
        interp[*c] = cpy;
        (*c)++;
    }

    interp[*c] = NULL;
    return interp;
}

static char **get_interp(uint32_t sid, int *c) {
    char *tmp = kmalloc(11);
    int size = 0;
    int to_read = 10;

    int file_size = fu_file_get_size(sid);
    if (file_size < 10)
        to_read = file_size;

    while (1) {
        fu_file_read(sid, tmp + size, size + 2, to_read);
        for (int i = size; i < size + to_read; i++) {
            if (tmp[i] == '\n') {
                tmp[i] = '\0';
                to_read = 0;
            }
        }
        if (to_read == 0)
            break;
        size += to_read;
        if (size + 2 + to_read > file_size)
            to_read = file_size - size - 2;
        if (to_read <= 0) {
            tmp[size] = '\0';
            break;
        }
        tmp = krealloc(tmp, size + to_read + 1);
    }

    char **interp = split_interp(tmp, c);
    kfree(tmp);

    return interp;
}

int run_ifexist_full(runtime_args_t args, int *pid_ptr) {
    if (pid_ptr != NULL)
        *pid_ptr = -1;

    if (args.path == NULL) {
        fd_printf(2, "[run_ifexist] path is NULL\n");
        return -1;
    }

    static uint32_t deluge_sid = SID_NULL;

    if (IS_SID_NULL(deluge_sid)) {
        deluge_sid = fu_path_to_sid(SID_ROOT, ELF_INTERP);
        if (IS_SID_NULL(deluge_sid)) {
            fd_printf(2, "[run_ifexist] interpreter not found: %s\n", ELF_INTERP);
            return -1;
        }
    }

    uint32_t sid = fu_path_to_sid(SID_ROOT, args.path);
    if (!fu_is_file(sid)) {
        fd_printf(2, "[run_ifexist] path not found: %s\n", args.path);
        return -1;
    }

    uint8_t magic[4];
    fu_file_read(sid, magic, 0, 4);

    char **nargv = NULL;

    if (magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F') {
        if (str_ncmp("/bin/sys/", args.path, 9) == 0) {
            nargv = kcalloc_ask(args.argc + 1, sizeof(char *));
            for (int i = 0; i < args.argc; i++) {
                nargv[i] = kmalloc_ask(str_len(args.argv[i]) + 1);
                str_cpy(nargv[i], args.argv[i]);
            }
            sid = fu_path_to_sid(SID_ROOT, args.path);
        } else {
            args.argc += 3;
            nargv = kcalloc_ask(args.argc + 1, sizeof(char *));

            nargv[0] = kmalloc_ask(4);
            str_cpy(nargv[0], "dlg");

            nargv[1] = kmalloc_ask(3);
            str_cpy(nargv[1], "-e");

            nargv[2] = kmalloc_ask(str_len(args.path) + 1);
            str_cpy(nargv[2], args.path);

            for (int i = 3; i < args.argc; i++) {
                nargv[i] = kmalloc_ask(str_len(args.argv[i-3]) + 1);
                str_cpy(nargv[i], args.argv[i-3]);
            }
            sid = deluge_sid;
        }
    } else {
        char **interp = NULL;
        int c;

        if (magic[0] == '#' && magic[1] == '!' && magic[2] == '/')
            interp = get_interp(sid, &c);
        else if (args.envp != NULL) {
            for (int i = 0; args.envp[i] != NULL; i++) {
                if (str_ncmp(args.envp[i], ENV_INTERP"=", str_len(ENV_INTERP)+1))
                    continue;
                interp = split_interp(args.envp[i] + str_len(ENV_INTERP) + 1, &c);
            }
            if (interp == NULL) {
                fd_printf(2, "[run_ifexist] %s: unknown file type\n", args.path);
                return -1;
            }
        }

        nargv = kcalloc_ask(args.argc + c + 3, sizeof(char *));

        nargv[0] = kmalloc_ask(4);
        str_cpy(nargv[0], "dlg");

        for (int i = 0; i < c; i++)
            nargv[i+1] = interp[i];
        kfree(interp);

        nargv[c+1] = kmalloc_ask(str_len(args.path) + 1);
        str_cpy(nargv[c+1], args.path);

        for (int i = 1; i < args.argc; i++) {
            nargv[i+c+1] = kmalloc_ask(str_len(args.argv[i]) + 1);
            str_cpy(nargv[i+c+1], args.argv[i]);
        }
        args.argc += c + 1;
        sid = deluge_sid;
    }

    char **nenv = dup_envp(args.envp);

    if (args.sleep_mode == 3) {
        syscall_mem_free_all(syscall_process_pid());
        return syscall_elf_exec(sid, args.argc, nargv, nenv);
    }

    int pid = syscall_process_create(syscall_elf_exec, 0, 4,
            (uint32_t []) {sid, args.argc, (uint32_t) nargv, (uint32_t) nenv}
    );

    if (pid_ptr != NULL)
        *pid_ptr = pid;
    if (pid == -1)
        return -1;

    fm_declare_child(pid);

    if (args.sleep_mode == 2)
        return 0;
    if (args.sleep_mode == 0)
        return (syscall_process_wakeup(pid, 0), 0);

    syscall_process_wakeup(pid, 1);
    uint8_t ret;
    syscall_process_wait(pid, &ret, 0);
    return ret;
}
