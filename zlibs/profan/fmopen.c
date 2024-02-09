#include <syscall.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define FMOPEN_LIB_C

#include <filesys.h>
#include <profan.h>

#define MAX_OPENED 100
#define MAX_STDHIST 20

typedef struct {
    uint8_t *data;
    uint32_t size;
    uint32_t writed;
    int      wpid[20];
    int      wpcnt;
    int      refs;
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

int main(void) {
    opened = calloc_ask(MAX_OPENED, sizeof(opened_t));
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
    for (index = 3; index < MAX_OPENED; index++) {
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
    return index;
}

int fm_reopen(int fd, char *path) {
    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);

    fm_close(fd);

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
    }

    if (opened[fd].type == TYPE_RPIP || opened[fd].type == TYPE_WPIP) {
        opened[fd].pipe->refs--;
        if (!opened[fd].pipe->refs) {
            free(opened[fd].pipe->data);
            free(opened[fd].pipe);
        }
    }

    opened[fd].type = 0;
    return 0;
}

int fm_read(int fd, void *buf, uint32_t size) {
    int read_count;
    uint32_t tmp;

    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);

    switch (opened[fd].type) {
        case TYPE_FILE:
            tmp = c_fs_cnt_get_size(c_fs_get_main(), opened[fd].sid);
            if (opened[fd].offset > tmp)
                return -1;
            if (opened[fd].offset + size > tmp)
                size = tmp - opened[fd].offset;
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
                opened[fd].pipe->data = realloc_ask(opened[fd].pipe->data, opened[fd].pipe->size);
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
    return opened[fd].offset;
}

int fm_dup(int fd) {
    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);
    if (!opened[fd].type)
        return -1;

    int index;
    for (index = 3; index < MAX_OPENED; index++) {
        if (!opened[index].type) break;
    }
    if (index == MAX_OPENED) {
        serial_debug("fm_dup: no more file descriptors\n");
        return -1;
    }

    opened[index].type = opened[fd].type;
    opened[index].offset = opened[fd].offset;
    opened[index].pid = opened[fd].pid;
    if (opened[index].type == TYPE_FCTF)
        opened[index].fctf = opened[fd].fctf;
    else if (opened[index].type == TYPE_RPIP) {
        opened[index].pipe = opened[fd].pipe;
        opened[index].pipe->refs++;
    } else if (opened[index].type == TYPE_WPIP) {
        opened[index].pipe = opened[fd].pipe;
        opened[index].pipe->refs++;
        for (int i = 0; i < opened[index].pipe->wpcnt; i++) {
            if (opened[index].pipe->wpid[i] == opened[index].pid)
                return index;
        }
        opened[index].pipe->wpid[opened[index].pipe->wpcnt++] = opened[index].pid;
    } else
        opened[index].sid = opened[fd].sid;
    return index;
}

int fm_dup2(int fd, int new_fd) {
    if (fd < 0 || fd >= MAX_OPENED || new_fd < 0 || new_fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);
    if (new_fd < 3)
        new_fd = fm_resol012(new_fd, -1);

    if (!opened[fd].type)
        return -1;

    if (opened[new_fd].type)
        fm_close(new_fd);

    opened[new_fd].type = opened[fd].type;
    opened[new_fd].offset = opened[fd].offset;
    if (opened[new_fd].type == TYPE_FCTF)
        opened[new_fd].fctf = opened[fd].fctf;
    else if (opened[new_fd].type == TYPE_RPIP) {
        opened[new_fd].pipe = opened[fd].pipe;
        opened[new_fd].pipe->refs++;
    } else if (opened[new_fd].type == TYPE_WPIP) {
        opened[new_fd].pipe = opened[fd].pipe;
        opened[new_fd].pipe->refs++;
        for (int i = 0; i < opened[new_fd].pipe->wpcnt; i++) {
            if (opened[new_fd].pipe->wpid[i] == opened[new_fd].pid)
                return 0;
        }
        opened[new_fd].pipe->wpid[opened[new_fd].pipe->wpcnt++] = opened[new_fd].pid;
    } else
        opened[new_fd].sid = opened[fd].sid;

    return 0;
}

int fm_pipe(int fd[2]) {
    int i1, i2, pid;
    pipe_t *pipe = calloc_ask(1, sizeof(pipe_t));

    for (i1 = 3; i1 < MAX_OPENED; i1++)
        if (!opened[i1].type) break;
    for (i2 = i1 + 1; i2 < MAX_OPENED; i2++)
        if (!opened[i2].type) break;
    if (i1 == MAX_OPENED || i2 == MAX_OPENED) {
        serial_debug("fm_pipe: no more file descriptors\n");
        return -1;
    }

    pid = c_process_get_pid();

    pipe->data = malloc_ask(1024);
    pipe->size = 1024;
    pipe->writed = 0;
    pipe->wpid[0] = pid;
    pipe->wpcnt = 1;
    pipe->refs = 2;

    opened[i1].type = TYPE_RPIP;
    opened[i1].pipe = pipe;
    opened[i1].offset = 0;
    opened[i1].pid = pid;

    opened[i2].type = TYPE_WPIP;
    opened[i2].pipe = pipe;
    opened[i2].offset = 0;
    opened[i2].pid = pid;

    fd[0] = i1;
    fd[1] = i2;

    return 0;
}

int fm_isfctf(int fd) {
    if (fd < 0 || fd >= MAX_OPENED)
        return -1;
    if (fd < 3)
        fd = fm_resol012(fd, -1);

    if (!opened[fd].type)
        return -1;
    return opened[fd].type == TYPE_FCTF;
}

void fm_clean(void) {
    int fd_free = 0;

    for (int i = 0; i < stdhist_len; i++) {
        if (c_process_get_state(stdhist[i].pid) < 4)
            continue;
        printf("fm_clean: stdhist %d (pid: %d)\n", i, stdhist[i].pid);
        fm_close(stdhist[i].fd[0]);
        fm_close(stdhist[i].fd[1]);
        fm_close(stdhist[i].fd[2]);
        memmove(stdhist + i, stdhist + i + 1, (stdhist_len - i - 1) * sizeof(stdhist_t));
        stdhist_len--;
        i--;
    }

    for (int i = 3; i < MAX_OPENED; i++) {
        if (!opened[i].type) continue;
        if (c_process_get_state(opened[i].pid) < 4)
            continue;
        printf("fm_clean: opened %d (pid: %d) [WARNING]\n", i, opened[i].pid);
        fm_close(i);
    }

    for (int i = 3; i < MAX_OPENED; i++) {
        if (!opened[i].type) {
            fd_free++;
            continue;
        }
    }

    printf("fm_clean: %d file descriptors are free\n", fd_free);
}

int fm_add_stdhist(int fd, int pid) {
    if (stdhist_len >= MAX_STDHIST) {
        for (int i = 0; i < stdhist_len; i++) {
            if (c_process_get_state(stdhist[i].pid) < 4)
                continue;
            for (int j = 0; j < 3; j++) {
                if (opened[stdhist[i].fd[j]].pid != stdhist[i].pid)
                    continue;
                fm_close(stdhist[i].fd[j]);
            }
            memmove(stdhist + i, stdhist + i + 1, (stdhist_len - i - 1) * sizeof(stdhist_t));
            stdhist_len--;
            i--;
        }
    }

    if (stdhist_len >= MAX_STDHIST) {
        printf("fm_add_stdhist: no more space in stdhist\n");
        return -1;
    }

    stdhist[stdhist_len].pid = pid;
    for (int i = 0; i < 3; i++) {
        stdhist[stdhist_len].fd[i] = fm_dup(i + 3);
        opened[stdhist[stdhist_len].fd[i]].pid = pid;
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
    return res;
}

int fm_resol012(int fd, int pid) {
    int key, maj;

    if (fd < 0 || fd >= 3)
        return -1;

    if (pid < 0)
        pid = c_process_get_pid();

    if (pid > stdhist[stdhist_len - 1].pid)
        return fm_add_stdhist(fd, pid);

    for (int i = 0; i < stdhist_len; i++) {
        if (stdhist[i].pid == pid) {
            return stdhist[i].fd[fd];
        }
    }

    return fm_add_stdhist(fd, pid);
}
