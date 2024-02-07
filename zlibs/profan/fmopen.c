#include <syscall.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define FMOPEN_LIB_C

#include <filesys.h>

#define MAX_OPENED 100
#define MAX_STDHIST 20

typedef struct {
    uint8_t *data;
    uint32_t size;
    uint32_t writed;
    int      wpid[20];
    int      wpcnt;
} pipe_t;

typedef struct {
    union {
        sid_t    sid;
        pipe_t  *pipe;
        int    (*fctf)(void *, uint32_t, uint32_t, uint8_t);
    };
    int      type;
    int      pid;
    uint32_t offset;
} opened_t;

typedef struct {
    int      fd[3];
    int      pid;
} stdhist_t;

opened_t    *opened;
stdhist_t   *stdhist;
int          stdhist_len;

#define TYPE_FILE 1
#define TYPE_FCTF 2
#define TYPE_RPIP 3
#define TYPE_WPIP 4

int fm_add_stdhist(int fd, int pid);
int fm_resol012(int fd, int pid);
int fm_reopen(int fd, char *path);
int fm_close(int fd);

void debug_print(char *frm, ...);

int main(void) {
    opened = calloc_ask(MAX_OPENED - 3, sizeof(opened_t));
    stdhist = calloc_ask(MAX_STDHIST, sizeof(stdhist_t));
    stdhist_len = 0;

    fm_reopen(3, "/dev/kb");
    fm_reopen(4, "/dev/kterm");
    fm_reopen(5, "/dev/kterm");

    return 0;
}

int fm_open(char *path) {
    sid_t sid = fu_path_to_sid(ROOT_SID, path);
    if (IS_NULL_SID(sid)) {
        printf("fm_open: %s not found\n", path);
        return -1;
    }

    if (!fu_is_fctf(sid) && !fu_is_file(sid)) {
        printf("fm_open: %s is not a file\n", path);
        return -1;
    }

    int index;
    for (index = 0; index < MAX_OPENED; index++) {
        if (!opened[index].type) break;
    }
    if (index == MAX_OPENED) {
        printf("fm_open: no more file descriptors\n");
        return -1;
    }

    opened[index].pid = c_process_get_pid();
    opened[index].type = fu_is_fctf(sid) ? TYPE_FCTF : TYPE_FILE;
    opened[index].offset = 0;
    opened[index].pipe = NULL;
    if (opened[index].type == TYPE_FCTF)
        opened[index].fctf = (void *) fu_fctf_get_addr(sid);
    else
        opened[index].sid = sid;
    return index + 3;
}

int fm_reopen(int fd, char *path) {
    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);

    fm_close(fd);
    fd -= 3;

    sid_t sid = fu_path_to_sid(ROOT_SID, path);
    if (IS_NULL_SID(sid)) {
        printf("fm_reopen: %s not found\n", path);
        return -1;
    }

    if (!fu_is_fctf(sid) && !fu_is_file(sid)) {
        printf("fm_reopen: %s is not a file\n", path);
        return -1;
    }

    opened[fd].pid = c_process_get_pid();
    opened[fd].type = fu_is_fctf(sid) ? TYPE_FCTF : TYPE_FILE;
    opened[fd].offset = 0;
    if (opened[fd].type == TYPE_FCTF)
        opened[fd].fctf = (void *) fu_fctf_get_addr(sid);
    else
        opened[fd].sid = sid;
    return 0;
}

int fm_close(int fd) {
    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);
    fd -= 3;

    if (!opened[fd].type)
        return -1;

    if (opened[fd].type == TYPE_WPIP) {
        for (int i = 0; i < opened[fd].pipe->wpcnt; i++) {
            if (opened[fd].pipe->wpid[i] == opened[fd].pid) {
                memmove(opened[fd].pipe->wpid + i, opened[fd].pipe->wpid + i + 1, (opened[fd].pipe->wpcnt - i - 1) * sizeof(int));
                opened[fd].pipe->wpcnt--;
                break;
            }
        }
        if (!opened[fd].pipe->wpcnt) {
            free(opened[fd].pipe->data);
            free(opened[fd].pipe);
        }
    }

    opened[fd].type = 0;
    return 0;
}

int fm_read(int fd, void *buf, uint32_t size) {
    int read_count;
    int tmp;

    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);
    fd -= 3;

    switch (opened[fd].type) {
        case TYPE_FILE:
            if (opened[fd].offset + size > c_fs_cnt_get_size(c_fs_get_main(), opened[fd].sid))
                return -1;
            read_count = c_fs_cnt_read(c_fs_get_main(), opened[fd].sid, buf, opened[fd].offset, size) ? 0 : size;
            break;
        case TYPE_FCTF:
            read_count = opened[fd].fctf(buf, opened[fd].offset, size, 1);
            break;
        case TYPE_RPIP:
            while (opened[fd].pipe->writed <= opened[fd].offset) {
                tmp = opened[fd].pipe->wpcnt;
                for (int i = 0; i < opened[fd].pipe->wpcnt; i++) {
                    if (c_process_get_state(opened[fd].pipe->wpid[i]) >= 4)
                        tmp--;
                }
                if (!tmp) break;
                c_process_sleep(c_process_get_pid(), 10);
            }
            read_count = opened[fd].pipe->writed - opened[fd].offset;
            if (read_count > (int) size)
                read_count = size;
            memcpy(buf, opened[fd].pipe->data + opened[fd].offset, read_count);
            break;
        default:
            return -1;
    }
    opened[fd].offset += read_count;
    return read_count;
}

int fm_write(int fd, void *buf, uint32_t size) {
    int write_count;

    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);
    fd -= 3;

    // debug_print("fm_write: %d %d\n", fd, opened[fd].type);

    switch (opened[fd].type) {
        case TYPE_FILE:
            if (opened[fd].offset + size > c_fs_cnt_get_size(c_fs_get_main(), opened[fd].sid))
                c_fs_cnt_set_size(c_fs_get_main(), opened[fd].sid, opened[fd].offset + size);
            write_count = c_fs_cnt_write(c_fs_get_main(), opened[fd].sid, buf, opened[fd].offset, size) ? 0 : size;
            break;
        case TYPE_FCTF:
            write_count = opened[fd].fctf(buf, opened[fd].offset, size, 0);
            break;
        case TYPE_WPIP:
            if (opened[fd].pipe->writed + size > opened[fd].pipe->size) {
                opened[fd].pipe->size = opened[fd].pipe->writed + size;
                opened[fd].pipe->data = realloc(opened[fd].pipe->data, opened[fd].pipe->size);
            }
            memcpy(opened[fd].pipe->data + opened[fd].pipe->writed, buf, size);
            opened[fd].pipe->writed += size;
            write_count = size;
            break;
        default:
            return -1;
    }

    opened[fd].offset += write_count;
    return write_count;
}

int fm_lseek(int fd, int offset, int whence) {
    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);
    fd -= 3;
    if (!opened[fd].type)
        return -1;

    switch (whence) {
        case SEEK_SET:
            opened[fd].offset = offset;
            break;
        case SEEK_CUR:
            opened[fd].offset += offset;
            break;
        case SEEK_END:
            if (opened[fd].type != TYPE_FILE)
                return -1;
            opened[fd].offset = c_fs_cnt_get_size(c_fs_get_main(), opened[fd].sid) + offset;
            break;
        default:
            return -1;
    }
    return opened[fd].offset;
}

int fm_tell(int fd) {
    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);
    fd -= 3;
    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    return opened[fd].offset;
}

int fm_dup(int fd) {
    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);
    fd -= 3;
    if (!opened[fd].type)
        return -1;

    int index;
    for (index = 0; index < MAX_OPENED; index++) {
        if (!opened[index].type) break;
    }
    if (index == MAX_OPENED) {
        debug_print("fm_dup: no more file descriptors\n");
        return -1;
    }

    opened[index].type = opened[fd].type;
    opened[index].offset = opened[fd].offset;
    opened[index].pid = opened[fd].pid;
    if (opened[index].type == TYPE_FCTF)
        opened[index].fctf = opened[fd].fctf;
    else if (opened[index].type == TYPE_RPIP)
        opened[index].pipe = opened[fd].pipe;
    else if (opened[index].type == TYPE_WPIP) {
        opened[index].pipe = opened[fd].pipe;
        opened[index].pipe->wpid[opened[index].pipe->wpcnt++] = opened[index].pid;
    } else
        opened[index].sid = opened[fd].sid;
    debug_print("fm_dup: %d %d\n", fd + 3, index + 3);
    return index + 3;
}

int fm_dup2(int fd, int new_fd) {
    if (fd < 0 || fd >= MAX_OPENED || new_fd < 0 || new_fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);
    if (new_fd < 3)
        new_fd = fm_resol012(new_fd, -1);
    fd -= 3;
    new_fd -= 3;
    if (!opened[fd].type)
        return -1;

    if (opened[new_fd].type)
        fm_close(new_fd + 3);

    opened[new_fd].type = opened[fd].type;
    opened[new_fd].offset = opened[fd].offset;
    if (opened[new_fd].type == TYPE_FCTF)
        opened[new_fd].fctf = opened[fd].fctf;
    else if (opened[new_fd].type == TYPE_RPIP)
        opened[new_fd].pipe = opened[fd].pipe;
    else if (opened[new_fd].type == TYPE_WPIP) {
        opened[new_fd].pipe = opened[fd].pipe;
        opened[new_fd].pipe->wpid[opened[new_fd].pipe->wpcnt++] = opened[new_fd].pid;
    } else
        opened[new_fd].sid = opened[fd].sid;

    debug_print("fm_dup2: %d %d -> pid: %d\n", fd + 3, new_fd + 3, opened[new_fd].pid);

    return 0;
}

int fm_pipe(int fd[2]) {
    int i1, i2, pid;
    pipe_t *pipe = calloc_ask(1, sizeof(pipe_t));
    
    for (i1 = 0; i1 < MAX_OPENED; i1++)
        if (!opened[i1].type) break;
    for (i2 = i1 + 1; i2 < MAX_OPENED; i2++)
        if (!opened[i2].type) break;
    if (i1 == MAX_OPENED || i2 == MAX_OPENED) {
        debug_print("fm_pipe: no more file descriptors\n");
        return -1;
    }

    pid = c_process_get_pid();

    pipe->data = malloc_ask(1024);
    pipe->size = 1024;
    pipe->writed = 0;
    pipe->wpid[0] = pid;
    pipe->wpcnt = 1;

    opened[i1].type = TYPE_RPIP;
    opened[i1].pipe = pipe;
    opened[i1].offset = 0;
    opened[i1].pid = pid;

    opened[i2].type = TYPE_WPIP;
    opened[i2].pipe = pipe;
    opened[i2].offset = 0;
    opened[i2].pid = pid;

    fd[0] = i1 + 3;
    fd[1] = i2 + 3;

    return 0;
}

int fm_isfile(int fd) {
    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);
    fd -= 3;
    if (!opened[fd].type)
        return -1;
    return opened[fd].type == TYPE_FILE;
}

void fm_debug(int fd) {
    if (fd < 0 || fd >= MAX_OPENED) {
        printf("stdhist_len: %d\n", stdhist_len);
        for (int i = 0; i < stdhist_len; i++) {
            printf("pid: %d, fd: %d %d %d\n", stdhist[i].pid, stdhist[i].fd[0], stdhist[i].fd[1], stdhist[i].fd[2]);
        }
        printf("opened:\n");
        for (int i = 0; i < MAX_OPENED - 3; i++) {
            if (!opened[i].type) continue;
            goto print;
            next:
        }
    }
    if (fd < 3)
        fd = fm_resol012(fd, -1);
    fd -= 3;
    print:
    if (!opened[fd].type)
        printf("fd %d is not opened\n", fd + 3);
    else if (opened[fd].type == TYPE_FILE)
        printf("fd: %d, sid: d%ds%d, type: %d, offset: %d, pid: %d\n", fd + 3, opened[fd].sid.device, opened[fd].sid.sector, opened[fd].type, opened[fd].offset, opened[fd].pid);
    else if (opened[fd].type == TYPE_FCTF)
        printf("fd: %d, fctf: %p, type: %d, offset: %d, pid: %d\n", fd + 3, opened[fd].fctf, opened[fd].type, opened[fd].offset, opened[fd].pid);
    else if (opened[fd].type == TYPE_RPIP || opened[fd].type == TYPE_WPIP)
        printf("fd: %d, pipe: %p, type: %d, offset: %d, pid: %d\n", fd + 3, opened[fd].pipe, opened[fd].type, opened[fd].offset, opened[fd].pid);
    else
        printf("fd: %d, type: %d, offset: %d, pid: %d\n", fd + 3, opened[fd].type, opened[fd].offset, opened[fd].pid);
    if (fd < - 3)
        goto next;
}

void fm_clean(void) {
    int count012 = 0;
    int countother = 0;

    for (int i = 0; i < stdhist_len; i++) {
        if (c_process_get_state(stdhist[i].pid) < 4)
            continue;
        fm_close(stdhist[i].fd[0]);
        fm_close(stdhist[i].fd[1]);
        fm_close(stdhist[i].fd[2]);
        memmove(stdhist + i, stdhist + i + 1, (stdhist_len - i - 1) * sizeof(stdhist_t));
        count012++;
        stdhist_len--;
    }
    for (int i = 0; i < MAX_OPENED - 3; i++) {
        if (!opened[i].type) continue;
        if (c_process_get_state(opened[i].pid) < 4)
            continue;
        fm_close(i + 3);
        countother++;
    }
    printf("fm_clean: %d stdhist, %d other\n", count012, countother);
}

int fm_add_stdhist(int fd, int pid) {
    if (stdhist_len == MAX_STDHIST) {
        for (int i = 0; i < stdhist_len; i++) {
            if (c_process_get_state(stdhist[i].pid) < 4)
                continue;
            fm_close(stdhist[i].fd[0]);
            fm_close(stdhist[i].fd[1]);
            fm_close(stdhist[i].fd[2]);
            debug_print("fm_add_stdhist: pid %d has been closed\n", stdhist[i].pid);
            memmove(stdhist + i, stdhist + i + 1, (stdhist_len - i - 1) * sizeof(stdhist_t));
            stdhist_len--;
        }
    }
    if (stdhist_len == MAX_STDHIST) {
        printf("fm_add_stdhist: no more space in stdhist\n");
        return -1;
    }

    stdhist[stdhist_len].pid = pid;
    for (int i = 0; i < 3; i++) {
        stdhist[stdhist_len].fd[i] = fm_dup(i + 3);
        opened[stdhist[stdhist_len].fd[i] - 3].pid = pid;
    }

    int res = stdhist[stdhist_len].fd[fd];

    stdhist_len++;

    // sort stdhist
    for (int i = stdhist_len - 1; i > 0; i--) {
        if (stdhist[i].pid < stdhist[i - 1].pid) {
            stdhist_t tmp = stdhist[i];
            stdhist[i] = stdhist[i - 1];
            stdhist[i - 1] = tmp;
        }
    }

    debug_print("fm_add_stdhist (pid: %d, fd: %d): %d (type: %d)\n", opened[res - 3].pid, fd, res, opened[res - 3].type);

    return res;
}

int fm_resol012(int fd, int pid) {
    if (fd < 0 || fd >= 3)
        return -1;

    if (pid < 0)
        pid = c_process_get_pid();

    int key = stdhist_len / 2;
    int maj = key;
    // use dichotomy to find the right stdhist
    do {
        if (stdhist[key].pid == pid) {
            // debug_print("fm_resol012 (pid: %d, fd: %d): %d\n", pid, fd, stdhist[key].fd[fd]);
            return stdhist[key].fd[fd];
        } else if (stdhist[key].pid < pid) {
            maj = (maj + 1) / 2;
            key += maj;
        } else {
            maj = (maj + 1) / 2;
            key -= maj;
        }
        // debug_print("fm_resol012: %d %d %d\n", key, maj, stdhist[key].pid);
    } while (!(key <= 0 || key >= stdhist_len ||
        (stdhist[key].pid < pid && stdhist[key + 1].pid > pid))
    );

    return fm_add_stdhist(fd, pid);
}

void debug_print(char *frm, ...) {
    va_list args;
    va_start(args, frm);
    char *str = malloc(1024);
    vsprintf(str, frm, args);
    c_serial_print(SERIAL_PORT_A, str);
    free(str);
    va_end(args);
}
