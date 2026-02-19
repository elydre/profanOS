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

#include <drivers/keyboard.h>
#include <kernel/butterfly.h>
#include <kernel/snowflake.h>
#include <kernel/process.h>
#include <minilib.h>
#include <system.h>

#include <modules/filesys.h>
#include <profan.h> // for runtime_args_t
#include <fcntl.h>  // for flags

#define DEFAULT_KB "/zada/keymap/azerty.map"
#define ELF_INTERP "/bin/x/deluge.elf"
#define ENV_INTERP "DEFRUN"

// input() setings
#define FIRST_L 15
#define SLEEP_T 20
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

static int userspace_reporter(char *message) {
    int len = 0;
    while (message[len] != '\0')
        len++;
    return fm_write(2, message, len) == len ? 0 : -1;
}

int __init(void) {
    kb_map = NULL;

    if (profan_kb_load_map(DEFAULT_KB)) {
        sys_warning("[profan module] failed to load default keymap");
        return 1;
    }

    sys_set_reporter(userspace_reporter);

    return 0;
}

int profan_kb_load_map(char *path) {
    uint32_t sid = kfu_path_to_sid(SID_ROOT, path);

    if (!kfu_is_file(sid))
        return 1;

    int file_size = fs_cnt_get_size(sid);
    char *file_content = malloc(file_size + 1);

    fs_cnt_read(sid, file_content, 0, file_size);
    file_content[file_size] = '\0';

    if (str_ncmp(file_content, "#KEYMAP", 7)) {
        free(file_content);
        return 1;
    }

    char *tmp = calloc(128);

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
    if (scancode > 64 || kb_map == NULL)
        return '\0';
    if (shift)
        return kb_map[scancode * 2 + 1];
    return kb_map[scancode * 2];
}

static void fd_putstr(int fd, const char *str) {
    fm_write(fd, (void *) str, str_len(str));
}

static void fd_putchar(int fd, char c) {
    fm_write(fd, (void *) &c, 1);
}

char *profan_input_keyboard(int *size, char *term_path) {
    int fd = fm_open(term_path, O_RDWR);
    if (fd < 0) {
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
        process_sleep(process_get_pid(), SLEEP_T);
        sc = kb_get_scancode();

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

    buffer = realloc(buffer, buffer_actual_size + 1);
    if (size)
        *size = buffer_actual_size;
    return buffer;
}

#define alloc_arg(size, pid) ((void *) mem_alloc(size, 5, pid))

static inline void *calloc_arg(size_t size, int pid) {
    void *ptr = alloc_arg(size, pid);

    if (ptr == NULL)
        return NULL;

    mem_set(ptr, 0, size);

    return ptr;
}

static char **dup_envp(char **envp, char *wd, int pid) {
    if (envp == NULL)
        return calloc_arg(sizeof(char *), pid);
    int envc, size = 0;

    for (envc = 0; envp[envc] != NULL; envc++)
        size += (wd && str_ncmp(envp[envc], "PWD=", 4) == 0) ?
                str_len(wd) + 5 : str_len(envp[envc]) + 1;

    size += (envc + 1) * sizeof(char *);

    char **nenvp = calloc_arg(size, pid);

    char *nenvp_start = (char *) nenvp + (envc + 1) * sizeof(char *);

    for (int i = 0; envp[i] != NULL; i++) {
        if (!wd || str_ncmp(envp[i], "PWD=", 4)) {
            for (envc = 0; envp[i][envc] != '\0'; envc++)
                nenvp_start[envc] = envp[i][envc];
            nenvp[i] = nenvp_start;
            nenvp_start += envc + 1;
            continue;
        }
        nenvp[i] = nenvp_start;
        str_copy(nenvp_start, "PWD=");
        str_copy(nenvp_start + 4, wd);
        nenvp_start += str_len(wd) + 5;
    }

    return nenvp;
}

static char **split_interp(char *tmp, int *c) {
    char **interp = calloc(sizeof(char *));
    *c = 0;

    for (int from, i = 0; tmp[i];) {
        while (tmp[i] == ' ' || tmp[i] == '\t' || tmp[i] == '\r')
            i++;
        from = i;
        while (tmp[i] && tmp[i] != ' ' && tmp[i] != '\t' && tmp[i] != '\r')
            i++;
        if (i == from)
            break;
        interp = realloc(interp, (*c + 2) * sizeof(char *));
        char *cpy = alloc_arg(i - from + 1, 0);
        mem_copy(cpy, tmp + from, i - from);
        cpy[i - from] = '\0';
        interp[*c] = cpy;
        (*c)++;
    }

    interp[*c] = NULL;
    return interp;
}

static char **get_interp(uint32_t sid, int *c) {
    char *tmp = malloc(11);
    int size = 0;
    int to_read = 10;

    int file_size = fs_cnt_get_size(sid);
    if (file_size < 10)
        to_read = file_size;

    while (1) {
        fs_cnt_read(sid, tmp + size, size + 2, to_read);
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
        tmp = realloc(tmp, size + to_read + 1);
    }

    char **interp = split_interp(tmp, c);
    free(tmp);

    return interp;
}

enum {
    FILE_RAWELF,
    FILE_DYNELF,
    FILE_SHEBANG,
    FILE_FBANG,
    FILE_OTHER
};

int run_ifexist(runtime_args_t *args, int *pid_ptr) {
    if (pid_ptr != NULL)
        *pid_ptr = -1;

    // safety checks
    if (args->path == NULL) {
        sys_warning("[run_ifexist] no path provided");
        return -1;
    }

    static uint32_t deluge_sid = SID_NULL;

    if (SID_IS_NULL(deluge_sid)) {
        deluge_sid = kfu_path_to_sid(SID_ROOT, ELF_INTERP);
        if (SID_IS_NULL(deluge_sid)) {
            sys_warning("[run_ifexist] elf interpreter not found: %s", ELF_INTERP);
            return -1;
        }
    }

    uint32_t sid = SID_NULL;

    if (args->path[0] != '/' && args->wd) {
        uint32_t cwd_sid = kfu_path_to_sid(SID_ROOT, args->wd);
        if (kfu_is_dir(cwd_sid))
            sid = kfu_path_to_sid(cwd_sid, args->path);
    } else {
        sid = kfu_path_to_sid(SID_ROOT, args->path);
    }

    if (!kfu_is_file(sid)) {
        sys_warning("[run_ifexist] path not found: %s", args->path);
        return -1;
    }

    // read the magic number
    uint8_t magic[4];
    if (fs_cnt_get_size(sid) < 4)
        mem_set(magic, 0, 4);
    else
        fs_cnt_read(sid, magic, 0, 4);

    int file_type, inter_offset = 0;

    char **interp = NULL;

    // get the interpreter
    if (magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F') {
        if (str_ncmp("/bin/x/", args->path, 7) == 0) {
            file_type = FILE_RAWELF;
        } else {
            file_type = FILE_DYNELF;
            sid = deluge_sid;
        }
    } else {
        if ((magic[0] == '#' || magic[0] == '>') && magic[1] == '!') {
            interp = get_interp(sid, &inter_offset);
            file_type = magic[0] == '#' ? FILE_SHEBANG : FILE_FBANG;
        } else {
            file_type = FILE_OTHER;

            if (args->envp) {
                int env_interp_len = str_len(ENV_INTERP) + 1;
                for (int i = 0; args->envp[i] != NULL; i++) {
                    if (str_ncmp(args->envp[i], ENV_INTERP"=", env_interp_len))
                        continue;
                    interp = split_interp(args->envp[i] + env_interp_len, &inter_offset);
                }
            }

            if (interp == NULL) {
                sys_warning("[run_ifexist] %s: no interpreter found, export '%s' to set one", args->path, ENV_INTERP);
                return -1;
            }
        }

        if (interp == NULL || inter_offset == 0) {
            sys_warning("[run_ifexist] %s: invalid shebang", args->path);
            return -1;
        }
        sid = deluge_sid;
    }

    int pid = process_get_pid();

    // generate arguments and environment
    char **nenv  = dup_envp(args->envp, args->wd, pid);
    char **nargv = calloc_arg((args->argc + 5 + inter_offset) * sizeof(char *), pid);

    // create the new process
    if (args->sleep_mode != 3) {
        pid = process_create(elf_exec, 0, 3,
                (uint32_t []) {sid, (uint32_t) nargv, (uint32_t) nenv}
        );

        if (pid == -1) {
            sys_warning("[run_ifexist] failed to create process for %s", args->path);
            return -1;
        }

        mem_alloc_fetch(nargv, pid);
        mem_alloc_fetch(nenv, pid);
    }

    if (pid_ptr != NULL)
        *pid_ptr = pid;

    // prepare the arguments
    if (file_type == FILE_RAWELF) {
        for (int i = 0; i < args->argc; i++) {
            nargv[i] = alloc_arg(str_len(args->argv[i]) + 1, pid);
            str_copy(nargv[i], args->argv[i]);
        }
    } else if (file_type == FILE_DYNELF) {
        nargv[0] = alloc_arg(4, pid);
        str_copy(nargv[0], "dlg");

        nargv[1] = alloc_arg(3, pid);
        str_copy(nargv[1], "-e");

        nargv[2] = alloc_arg(str_len(args->path) + 1, pid);
        str_copy(nargv[2], args->path);

        for (int i = 0; i < args->argc; i++) {
            nargv[i + 3] = alloc_arg(str_len(args->argv[i]) + 1, pid);
            str_copy(nargv[i + 3], args->argv[i]);
        }
    } else {
        int c = 0;

        nargv[c] = alloc_arg(4, pid);
        str_copy(nargv[c++], "dlg");

        if (file_type == FILE_FBANG) {
            nargv[c] = alloc_arg(3, pid);
            str_copy(nargv[c++], "-e");

            nargv[c++] = interp[0];

            if (args->argc) {
                nargv[c] = alloc_arg(str_len(args->argv[0]) + 1, pid);
                str_copy(nargv[c++], args->argv[0]);
            }
        }

        for (int i = file_type == FILE_FBANG; interp[i] != NULL; i++) {
            nargv[c++] = interp[i];
        }

        if (file_type != FILE_FBANG) {
            nargv[c] = alloc_arg(str_len(args->path) + 1, pid);
            str_copy(nargv[c++], args->path);
        }

        for (int i = 1; i < args->argc; i++) {
            nargv[c] = alloc_arg(str_len(args->argv[i]) + 1, pid);
            str_copy(nargv[c++], args->argv[i]);
        }

        sid = deluge_sid;
    }

    free(interp);

    // finalize the execution
    if (args->sleep_mode == 3) {
        mem_free_all(process_get_pid(), 1);
        return elf_exec(sid, nargv, nenv);
    }

    fm_declare_child(pid);

    if (args->sleep_mode == 2)
        return 0;
    if (args->sleep_mode == 0)
        return (process_wakeup(pid, 0), 0);

    uint8_t ret;

    process_wakeup(pid, 1);
    process_wait(pid, &ret, 0);

    return ret;
}

void *__module_func_array[] = {
    (void *) 0xF3A3C4D4, // magic
    profan_kb_load_map,
    profan_kb_get_char,
    profan_input_keyboard,
    run_ifexist
};
