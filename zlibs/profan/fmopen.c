#include <syscall.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define FMOPEN_LIB_C

#include <filesys.h>
#define MAX_OPENED 100
#define MAX_STDHIST 20

typedef struct {
    sid_t sid;
    int (*fctf)(void *, uint32_t, uint32_t, uint8_t);
    int type;
    int pid;
    uint32_t offset;
} opened_t;

typedef struct {
    int fd[3];
    int pid;
} stdhist_t;

opened_t  *opened;
stdhist_t *stdhist;
int stdhist_len;

#define TYPE_FILE 1
#define TYPE_FCTF 2

int fm_add_stdhist(int fd, int pid);
int fm_resol012(int fd, int pid);
int fm_reopen(int fd, char *path);

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

    opened[index].sid = sid;
    opened[index].pid = c_process_get_pid();
    opened[index].type = fu_is_fctf(sid) ? TYPE_FCTF : TYPE_FILE;
    opened[index].offset = 0;
    if (opened[index].type == TYPE_FCTF)
        opened[index].fctf = (void *) fu_fctf_get_addr(sid);
    return index + 3;
}

int fm_reopen(int fd, char *path) {
    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);
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

    opened[fd].sid = sid;
    opened[fd].type = fu_is_fctf(sid) ? TYPE_FCTF : TYPE_FILE;
    opened[fd].offset = 0;
    if (opened[fd].type == TYPE_FCTF)
        opened[fd].fctf = (void *) fu_fctf_get_addr(sid);
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

    opened[fd].type = 0;
    return 0;
}

int fm_read(int fd, void *buf, uint32_t size) {
    int read_count;

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

    opened[index].sid = opened[fd].sid;
    opened[index].type = opened[fd].type;
    opened[index].offset = opened[fd].offset;
    if (opened[index].type == TYPE_FCTF)
        opened[index].fctf = opened[fd].fctf;
    debug_print("fm_dup: %d %d\n", fd, index + 3);
    return index + 3;
}

int fm_dup2(int fd, int new_fd) {
    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);
    fd -= 3;
    if (!opened[fd].type)
        return -1;

    if (new_fd < 0 || new_fd >= MAX_OPENED)
        return -1;
    if (new_fd < 3)
        new_fd = fm_resol012(new_fd, -1);
    new_fd -= 3;

    opened[new_fd].sid = opened[fd].sid;
    opened[new_fd].type = opened[fd].type;
    opened[new_fd].offset = opened[fd].offset;
    if (opened[new_fd].type == TYPE_FCTF)
        opened[new_fd].fctf = opened[fd].fctf;
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

void fm_debug(void) {
    printf("stdhist_len: %d\n", stdhist_len);
    for (int i = 0; i < stdhist_len; i++) {
        printf("pid: %d, fd: %d %d %d\n", stdhist[i].pid, stdhist[i].fd[0], stdhist[i].fd[1], stdhist[i].fd[2]);
    }
    printf("opened:\n");
    for (int i = 0; i < MAX_OPENED - 3; i++) {
        if (!opened[i].type) continue;
        printf("fd: %d, sid: d%ds%d, type: %d, offset: %d\n", i + 3, opened[i].sid.device, opened[i].sid.sector, opened[i].type, opened[i].offset);
    }
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
    stdhist[stdhist_len].fd[0] = fm_dup(3);
    stdhist[stdhist_len].fd[1] = fm_dup(4);
    stdhist[stdhist_len].fd[2] = fm_dup(5);

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

    debug_print("fm_add_stdhist (pid: %d, fd: %d): %d (type: %d)\n", pid, fd, res, opened[res - 3].type);

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
    while (1) {
        if (stdhist[key].pid == pid) {
            debug_print("fm_resol012 (pid: %d, fd: %d): %d\n", pid, fd, stdhist[key].fd[fd]);
            return stdhist[key].fd[fd];
        } else if (stdhist[key].pid < pid) {
            maj = (maj + 1) / 2;
            key += maj;
        } else {
            maj = (maj + 1) / 2;
            key -= maj;
        }
        if (maj == 0 || key < 0 || key >= stdhist_len) {
            debug_print("fm_resol012: no stdhist found for pid %d\n", pid);
            break;
        }
    }
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
