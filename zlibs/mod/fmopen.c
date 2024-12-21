/*****************************************************************************\
|   === fmopen.c : 2024 ===                                                   |
|                                                                             |
|    Unix file manipulation compatibility layer as module          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#define FMOPEN_LIB_C

#define _SYSCALL_CREATE_STATIC
#include <profan/syscall.h>

#include <profan/filesys.h>
#include <profan/libmmq.h>
#include <profan.h>

#include <errno.h>
#include <fcntl.h> // for flags

#define PIPE_MAX     256
#define PIPE_MAX_REF 12

enum {
    TYPE_FREE = 0,
    TYPE_FILE,
    TYPE_FCTF,
    TYPE_PPRD, // read pipe
    TYPE_PPWR  // write pipe
};

typedef struct {
    uint32_t buffer_size;
    void    *buf;

    uint32_t current_size;
    uint32_t rd_offset;

    int readers[PIPE_MAX_REF];
    int writers[PIPE_MAX_REF];
} pipe_data_t;

typedef struct {
    uint8_t type;

    union {
        uint32_t     sid; // for file
        int        (*fctf)(int, void *, uint32_t, uint8_t);
        pipe_data_t *pipe;
    };

    union {
        uint32_t offset;  // file
        int      fctf_id; // fcft
    };
} fd_data_t;

pipe_data_t *open_pipes;

int main(void) {
    open_pipes = kcalloc_ask(PIPE_MAX, sizeof(pipe_data_t));
    return 0;
}

#define MAX_FD ((int)(0x1000 / sizeof(fd_data_t)))

static fd_data_t *fm_fd_to_data(int fd) {
    if (fd < 0 || fd >= MAX_FD)
        return NULL;
    return (fd_data_t *) (0xB0000000 + fd * sizeof(fd_data_t));
}

static fd_data_t *fm_get_free_fd(int *fd) {
    for (int i = 0; i < MAX_FD; i++) {
        fd_data_t *fd_data = fm_fd_to_data(i);
        if (fd_data->type == TYPE_FREE) {
            if (fd != NULL)
                *fd = i;
            return fd_data;
        }
    }
    if (fd != NULL)
        *fd = -1;
    return NULL;
}

static pipe_data_t *fm_get_free_pipe(void) {
    // free pipe with only dead processes
    for (int i = 0; i < PIPE_MAX; i++) {
        if (open_pipes[i].buf == NULL)
            continue;
        for (int j = 0; j < PIPE_MAX_REF; j++) {
            if (open_pipes[i].readers[j] != -1 && syscall_process_state(open_pipes[i].readers[j]) > 1)
                break;
            if (open_pipes[i].writers[j] != -1 && syscall_process_state(open_pipes[i].writers[j]) > 1)
                break;
            if (j == PIPE_MAX_REF - 1) {
                kfree(open_pipes[i].buf);
                open_pipes[i].buf = NULL;
            }
        }
    }

    for (int i = 0; i < PIPE_MAX; i++) {
        if (open_pipes[i].buf == NULL)
            return open_pipes + i;
    }
    return NULL;
}

static int fm_pipe_push_reader(pipe_data_t *pipe, int pid) {
    for (int i = 0; i < PIPE_MAX_REF; i++) {
        if (pipe->readers[i] == -1) {
            pipe->readers[i] = pid;
            return 0;
        }
    }
    return -1;
}

static int fm_pipe_push_writer(pipe_data_t *pipe, int pid) {
    for (int i = 0; i < PIPE_MAX_REF; i++) {
        if (pipe->writers[i] == -1) {
            pipe->writers[i] = pid;
            return 0;
        }
    }
    return -1;
}

int fm_close(int fd) {
    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL || fd_data->type == TYPE_FREE)
        return -EBADF;

    if (fd_data->type == TYPE_FCTF)
        fd_data->fctf(fd_data->fctf_id, NULL, 0, FCTF_CLOSE);

    else if (fd_data->type == TYPE_PPRD) {
        pipe_data_t *pipe = fd_data->pipe;
        int pid = syscall_process_pid();

        for (int i = 0; i < PIPE_MAX_REF; i++) {
            if (pipe->readers[i] != pid)
                continue;
            for (int j = i; j < PIPE_MAX_REF - 1; j++)
                pipe->readers[j] = pipe->readers[j + 1];
            pipe->readers[PIPE_MAX_REF - 1] = -1;
            break;
        }

        if (pipe->readers[0] == -1 && pipe->writers[0] == -1) {
            kfree(pipe->buf);
            pipe->buf = NULL;
        }
    }

    else if (fd_data->type == TYPE_PPWR) {
        pipe_data_t *pipe = fd_data->pipe;
        int pid = syscall_process_pid();

        for (int i = 0; i < PIPE_MAX_REF; i++) {
            if (pipe->writers[i] != pid)
                continue;
            for (int j = i; j < PIPE_MAX_REF - 1; j++)
                pipe->writers[j] = pipe->writers[j + 1];
            pipe->writers[PIPE_MAX_REF - 1] = -1;
            break;
        }

        if (pipe->readers[0] == -1 && pipe->writers[0] == -1) {
            kfree(pipe->buf);
            pipe->buf = NULL;
        }
    }

    fd_data->type = TYPE_FREE;
    return 0;
}

int fm_reopen(int fd, const char *abs_path, int flags) {
    // return fd or -errno

    fm_close(fd);

    fd_data_t *fd_data;

    uint32_t sid = fu_path_to_sid(SID_ROOT, abs_path);

    if (IS_SID_NULL(sid) && (flags & O_CREAT)) {
        sid = fu_file_create(0, abs_path);
    }

    if (IS_SID_NULL(sid)) {
        return -ENOENT;
    }

    if (!fu_is_fctf(sid) && !fu_is_file(sid)) {
        return -EFTYPE;
    }

    if (fd < 0)
        fd_data = fm_get_free_fd(&fd);
    else
        fd_data = fm_fd_to_data(fd);

    // fd_printf(2, "fd: %d, sid: %x, flags: %x\n", fd, sid, flags);

    if (fd < 0) {
        return -EMFILE;
    }

    if (fu_is_file(sid)) {
        if (flags & O_TRUNC) {
            syscall_fs_set_size(NULL, sid, 0);
        }
        fd_data->type = TYPE_FILE;
        fd_data->sid = sid;
        if (flags & O_APPEND) {
            fd_data->offset = syscall_fs_get_size(NULL, sid);
        } else {
            fd_data->offset = 0;
        }
    } else {
        fd_data->type = TYPE_FCTF;
        fd_data->fctf = (void *) fu_fctf_get_addr(sid);

        fd_data->fctf_id = fd_data->fctf(0, NULL, 0, FCTF_OPEN);
        if (fd_data->fctf_id < 0) {
            fd_data->type = TYPE_FREE;
            return -EACCES;
        }
    }

    return fd;
}

int fm_read(int fd, void *buf, uint32_t size) {
    uint32_t tmp;
    int read_count;

    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL)
        return -EBADF;

    switch (fd_data->type) {
        case TYPE_FILE:
            tmp = syscall_fs_get_size(NULL, fd_data->sid);
            if (fd_data->offset > tmp)
                return -EINVAL;
            if (fd_data->offset + size > tmp)
                size = tmp - fd_data->offset;
            read_count = syscall_fs_read(NULL, fd_data->sid, buf, fd_data->offset, size) ? -1 : (int) size;
            break;

        case TYPE_FCTF:
            read_count = fd_data->fctf(fd_data->fctf_id, buf, size, FCTF_READ);
            if (read_count < 0)
                return -EIO;
            break;

        case TYPE_PPRD:
            pipe_data_t *pipe = fd_data->pipe;
            int pid = syscall_process_pid();

            while (pipe->current_size <= pipe->rd_offset && pipe->writers[0] != -1) {
                // fd_printf(2, "waiting for %d\n", pipe->writers[0]);
                syscall_process_sleep(pid, 10);
                for (int i = 0; i < PIPE_MAX_REF; i++) {
                    if (pipe->writers[i] == -1 || syscall_process_state(pipe->writers[i]) > 1)
                        continue;
                    for (int j = i; j < PIPE_MAX_REF - 1; j++)
                        pipe->writers[j] = pipe->writers[j + 1];
                    pipe->writers[PIPE_MAX_REF - 1] = -1;
                    break;
                }
            }

            if (pipe->current_size <= pipe->rd_offset)
                return 0;

            read_count = pipe->current_size - pipe->rd_offset;
            if (read_count > (int) size)
                read_count = size;

            mem_cpy(buf, pipe->buf + pipe->rd_offset, read_count);
            pipe->rd_offset += read_count;
            break;

        default:
            return -EBADF;
    }

    fd_data->offset += read_count;
    return read_count;
}

int fm_write(int fd, void *buf, uint32_t size) {
    int write_count;

    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL)
        return -EBADF;

    switch (fd_data->type) {
        case TYPE_FILE:
            if (fd_data->offset + size > syscall_fs_get_size(NULL, fd_data->sid))
                syscall_fs_set_size(NULL, fd_data->sid, fd_data->offset + size);
            write_count = syscall_fs_write(NULL, fd_data->sid, buf, fd_data->offset, size) ? 0 : size;
            break;

        case TYPE_FCTF:
            write_count = fd_data->fctf(fd_data->fctf_id, buf, size, FCTF_WRITE);
            if (write_count < 0)
                return -EIO;
            break;

        case TYPE_PPWR:
            pipe_data_t *pipe = fd_data->pipe;

            uint32_t new_size = pipe->current_size + size;

            if (new_size > pipe->buffer_size) {
                void *tmp = krealloc_ask(pipe->buf, new_size);
                if (tmp == NULL)
                    return -ENOMEM;
                pipe->buf = tmp;
                pipe->buffer_size = new_size;
            }

            mem_cpy(pipe->buf + pipe->current_size, buf, size);
            pipe->current_size += size;
            write_count = size;
            break;

        default:
            return -EBADF;
    }

    fd_data->offset += write_count;
    return write_count;
}

int fm_lseek(int fd, int offset, int whence) {
    fd_data_t *fd_data = fm_fd_to_data(fd);
    int new_offset;

    if (fd_data == NULL)
        return -EBADF;

    if (fd_data->type != TYPE_FILE)
        return -ESPIPE;

    new_offset = fd_data->offset;

    switch (whence) {
        case SEEK_SET:
            new_offset = offset;
            break;
        case SEEK_CUR:
            new_offset += offset;
            break;
        case SEEK_END:
            if (fd_data->type != TYPE_FILE)
                return -ESPIPE;
            new_offset = syscall_fs_get_size(NULL, fd_data->sid) + offset;
            break;
        default:
            return -EINVAL;
    }

    if (new_offset < 0)
        return -EINVAL;

    fd_data->offset = new_offset;

    return new_offset;
}

int fm_dup2(int fd, int new_fd) {
    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL || fd_data->type == TYPE_FREE)
        return -EBADF;

    fd_data_t *new_data = fm_fd_to_data(new_fd);

    if (new_data == NULL)
        return -EBADF;

    if (new_data->type != TYPE_FREE && fm_close(new_fd) < 0)
        return -EBADF;

    new_data->type = fd_data->type;

    switch (fd_data->type) {
        case TYPE_FILE:
            new_data->sid = fd_data->sid;
            new_data->offset = fd_data->offset;
            break;

        case TYPE_FCTF:
            new_data->fctf = fd_data->fctf;
            new_data->fctf_id = fd_data->fctf_id;
            break;

        case TYPE_PPRD:
            fm_pipe_push_reader(fd_data->pipe, syscall_process_pid());
            new_data->pipe = fd_data->pipe;
            break;

        case TYPE_PPWR:
            fm_pipe_push_writer(fd_data->pipe, syscall_process_pid());
            new_data->pipe = fd_data->pipe;
            break;

        default:
            return -EBADF;
    }

    return new_fd;
}

int fm_dup(int fd) {
    int new_fd;
    fm_get_free_fd(&new_fd);

    if (new_fd < 0)
        return -EMFILE;

    return fm_dup2(fd, new_fd);
}

int fm_pipe(int fd[2]) {
    fd_data_t *fd_data[2];

    pipe_data_t *pipe = fm_get_free_pipe();
    if (pipe == NULL)
        return -EMFILE;

    pipe->buf = kcalloc_ask(0x1000, 1);
    if (pipe->buf == NULL)
        return -ENOMEM;

    pipe->buffer_size = 0x1000;
    pipe->current_size = 0;
    pipe->rd_offset = 0;

    for (int i = 0; i < PIPE_MAX_REF; i++) {
        pipe->readers[i] = -1;
        pipe->writers[i] = -1;
    }

    int pid = syscall_process_pid();
    pipe->readers[0] = pid;
    pipe->writers[0] = pid;

    fd_data[0] = fm_get_free_fd(&fd[0]);
    if (fd_data[0] == NULL)
        return -EMFILE;
    fd_data[0]->type = TYPE_PPRD;

    fd_data[1] = fm_get_free_fd(&fd[1]);
    if (fd_data[1] == NULL) {
        fd_data[0]->type = TYPE_FREE;
        return -EMFILE;
    }
    fd_data[1]->type = TYPE_PPWR;

    fd_data[0]->pipe = pipe;
    fd_data[1]->pipe = pipe;

    return 0;
}

int fm_isfctf(int fd) {
    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL || fd_data->type == TYPE_FREE)
        return -EBADF;

    return fd_data->type == TYPE_FCTF;
}

int fm_isfile(int fd) {
    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL || fd_data->type == TYPE_FREE)
        return -EBADF;

    return fd_data->type == TYPE_FILE;
}

int fm_newfd_after(int fd) {
    // return the first free fd after fd

    for (int i = fd; i < MAX_FD; i++) {
        fd_data_t *fd_data = fm_fd_to_data(i);
        if (fd_data->type == TYPE_FREE)
            return i;
    }

    return -1;
}

int fm_declare_child(int pid) {
    // add pid to all pipes

    for (int i = 0; i < MAX_FD; i++) {
        fd_data_t *fd_data = fm_fd_to_data(i);

        if (!fd_data)
            continue;

        pipe_data_t *pipe = fd_data->pipe;

        if (fd_data->type == TYPE_PPRD && fm_pipe_push_reader(pipe, pid) < 0)
            return -1;

        if (fd_data->type == TYPE_PPWR && fm_pipe_push_writer(pipe, pid) < 0)
            return -1;
    }

    return 0;
}
