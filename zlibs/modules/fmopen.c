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

#include <kernel/butterfly.h>
#include <kernel/process.h>
#include <kernel/afft.h>
#include <minilib.h>
#include <system.h>

#include <unistd.h> // for SEEK_SET, SEEK_CUR, SEEK_END
#include <fcntl.h>  // for flags
#include <errno.h>

#define PIPE_MAX     256
#define PIPE_MAX_REF 12

enum {
    TYPE_FREE = 0,
    TYPE_FILE,
    TYPE_AFFT,
    TYPE_DIR,
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
    uint32_t sid;
    int flags;

    uint32_t offset;  // file and afft

    union {
        int          afft_id; // afft
        pipe_data_t *pipe;    // pipe
        char        *path;    // dir (for fchdir)
    };
} fd_data_t;

pipe_data_t *open_pipes;

#define MAX_FD ((int)(0x1000 / sizeof(fd_data_t)))

static inline fd_data_t *fm_fd_to_data(int fd) {
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
            if (open_pipes[i].readers[j] != -1 && process_info(open_pipes[i].readers[j], PROC_INFO_STATE, NULL) > 1)
                break;
            if (open_pipes[i].writers[j] != -1 && process_info(open_pipes[i].writers[j], PROC_INFO_STATE, NULL) > 1)
                break;
            if (j == PIPE_MAX_REF - 1) {
                free(open_pipes[i].buf);
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

    if (fd_data->type == TYPE_DIR)
        free(fd_data->path);

    else if (fd_data->type == TYPE_PPRD) {
        pipe_data_t *pipe = fd_data->pipe;
        int pid = process_get_pid();

        for (int i = 0; i < PIPE_MAX_REF; i++) {
            if (pipe->readers[i] != pid)
                continue;
            for (int j = i; j < PIPE_MAX_REF - 1; j++)
                pipe->readers[j] = pipe->readers[j + 1];
            pipe->readers[PIPE_MAX_REF - 1] = -1;
            break;
        }

        if (pipe->readers[0] == -1 && pipe->writers[0] == -1) {
            free(pipe->buf);
            pipe->buf = NULL;
        }
    }

    else if (fd_data->type == TYPE_PPWR) {
        pipe_data_t *pipe = fd_data->pipe;
        int pid = process_get_pid();

        for (int i = 0; i < PIPE_MAX_REF; i++) {
            if (pipe->writers[i] != pid)
                continue;
            for (int j = i; j < PIPE_MAX_REF - 1; j++)
                pipe->writers[j] = pipe->writers[j + 1];
            pipe->writers[PIPE_MAX_REF - 1] = -1;
            break;
        }

        if (pipe->readers[0] == -1 && pipe->writers[0] == -1) {
            free(pipe->buf);
            pipe->buf = NULL;
        }
    }

    fd_data->type = TYPE_FREE;
    return 0;
}

static uint32_t create_new_file(const char *path) {
    char *parent = NULL;
    char *cnt    = NULL;

    kfu_sep_path(path, &parent, &cnt);
    uint32_t sid = kfu_file_create(parent, cnt);

    free(parent);
    free(cnt);

    return sid;
}

int fm_reopen(int fd, const char *abs_path, int flags) {
    // return fd or -errno

    fm_close(fd);

    fd_data_t *fd_data;

    uint32_t sid = kfu_path_to_sid(SID_ROOT, abs_path);
    int access = flags & O_ACCMODE;

    if (IS_SID_NULL(sid)) {
        if (!(O_CREAT & flags))
            return -ENOENT;

        if (O_DIRECTORY & flags)
            return -ENOENT;

        sid = create_new_file(abs_path);
        if (IS_SID_NULL(sid))
            return -EACCES;
    } else {
        if (O_EXCL & flags)
            return -EEXIST;

        if (kfu_is_dir(sid)) {
            if (access != O_RDONLY ||
                flags & O_NODIR    ||
                flags & O_TRUNC    ||
                flags & O_APPEND   ||
                flags & O_CREAT
            ) return -EISDIR;
        } else if (flags & O_DIRECTORY) {
            return -ENOTDIR;
        }
    }

    if (!kfu_is_file(sid) && !kfu_is_afft(sid) && !kfu_is_dir(sid)) {
        return -EFTYPE;
    }

    if (fd < 0)
        fd_data = fm_get_free_fd(&fd);
    else
        fd_data = fm_fd_to_data(fd);

    if (fd < 0) {
        return -EMFILE;
    }

    fd_data->flags = flags;
    fd_data->sid = sid;

    if (kfu_is_file(sid)) {
        if (flags & O_TRUNC) {
            fs_cnt_set_size(sid, 0);
        }
        fd_data->type = TYPE_FILE;
        if (flags & O_APPEND) {
            fd_data->offset = fs_cnt_get_size(sid);
        } else {
            fd_data->offset = 0;
        }
    } else if (kfu_is_dir(sid)) {
        fd_data->type = TYPE_DIR;
        int len = str_len(abs_path);
        fd_data->path = malloc(len + 1);
        str_cpy(fd_data->path, abs_path);
    } else {
        fd_data->type = TYPE_AFFT;
        fd_data->afft_id = kfu_afft_get_id(sid);
        fd_data->offset = 0;

        if (fd_data->afft_id < 0) {
            fd_data->type = TYPE_FREE;
            sys_warning("[fm_reopen] Failed to get AFFT ID for %s", abs_path);
            return -EACCES;
        }
    }

    return fd;
}

int fm_pread(int fd, void *buf, uint32_t size, int offset) {
    int read_count, sync = 0;
    uint32_t tmp;

    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL)
        return -EBADF;

    if (offset < 0) {
        offset = fd_data->offset;
        sync = 1;
    }

    switch (fd_data->type) {
        case TYPE_DIR:
            return -EISDIR;

        case TYPE_FILE:
            tmp = fs_cnt_get_size(fd_data->sid);
            if ((uint32_t) offset > tmp)
                return -EINVAL;
            if (offset + size > tmp)
                size = tmp - offset;
            read_count = fs_cnt_read(fd_data->sid, buf, offset, size) ? -1 : (int) size;
            break;

        case TYPE_AFFT:
            read_count = afft_read(fd_data->afft_id, buf, offset, size);
            if (read_count < 0)
                return -EIO;
            break;

        case TYPE_PPRD:
            pipe_data_t *pipe = fd_data->pipe;
            int pid = process_get_pid();

            while (pipe->current_size <= pipe->rd_offset && pipe->writers[0] != -1) {
                // fd_printf(2, "waiting for %d\n", pipe->writers[0]);
                process_sleep(pid, 10);
                for (int i = 0; i < PIPE_MAX_REF; i++) {
                    if (pipe->writers[i] == -1 || process_info(pipe->writers[i], PROC_INFO_STATE, NULL) > 1)
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

            mem_copy(buf, pipe->buf + pipe->rd_offset, read_count);
            pipe->rd_offset += read_count;
            break;

        default:
            return -EBADF;
    }

    if (sync)
        fd_data->offset += read_count;

    return read_count;
}

int fm_pwrite(int fd, void *buf, uint32_t size, int offset) {
    int write_count, sync = 0;

    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL)
        return -EBADF;

    if (offset < 0) {
        offset = fd_data->offset;
        sync = 1;
    }

    switch (fd_data->type) {
        case TYPE_DIR:
            return -EISDIR;

        case TYPE_FILE:
            if (offset + size > fs_cnt_get_size(fd_data->sid))
                fs_cnt_set_size(fd_data->sid, offset + size);
            write_count = fs_cnt_write(fd_data->sid, buf, offset, size) ? 0 : size;
            break;

        case TYPE_AFFT:
            write_count = afft_write(fd_data->afft_id, buf, offset, size);
            if (write_count < 0)
                return -EIO;
            break;

        case TYPE_PPWR:
            pipe_data_t *pipe = fd_data->pipe;

            uint32_t new_size = pipe->current_size + size;

            if (new_size > pipe->buffer_size) {
                void *tmp = realloc(pipe->buf, new_size);
                if (tmp == NULL)
                    return -ENOMEM;
                pipe->buf = tmp;
                pipe->buffer_size = new_size;
            }

            mem_copy(pipe->buf + pipe->current_size, buf, size);
            pipe->current_size += size;
            write_count = size;
            break;

        default:
            return -EBADF;
    }

    if (sync)
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
            new_offset = fs_cnt_get_size(fd_data->sid) + offset;
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

    new_data->flags = fd_data->flags;
    new_data->type = fd_data->type;
    new_data->sid = fd_data->sid;

    switch (fd_data->type) {
        case TYPE_DIR:
            int len = str_len(fd_data->path);
            new_data->path = malloc(len + 1);
            str_cpy(new_data->path, fd_data->path);
            break;

        case TYPE_FILE:
            new_data->offset = fd_data->offset;
            break;

        case TYPE_AFFT:
            new_data->afft_id = fd_data->afft_id;
            new_data->offset = fd_data->offset;
            break;

        case TYPE_PPRD:
            fm_pipe_push_reader(fd_data->pipe, process_get_pid());
            new_data->pipe = fd_data->pipe;
            break;

        case TYPE_PPWR:
            fm_pipe_push_writer(fd_data->pipe, process_get_pid());
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

    pipe->buf = calloc(0x1000);
    if (pipe->buf == NULL)
        return -ENOMEM;

    pipe->buffer_size = 0x1000;
    pipe->current_size = 0;
    pipe->rd_offset = 0;

    for (int i = 0; i < PIPE_MAX_REF; i++) {
        pipe->readers[i] = -1;
        pipe->writers[i] = -1;
    }

    int pid = process_get_pid();
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

    fd_data[0]->flags = O_RDONLY;
    fd_data[1]->flags = O_WRONLY;

    fd_data[0]->pipe = pipe;
    fd_data[1]->pipe = pipe;

    fd_data[0]->sid = SID_NULL;
    fd_data[1]->sid = SID_NULL;

    return 0;
}

int fm_isafft(int fd) {
    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL || fd_data->type == TYPE_FREE)
        return -EBADF;

    return fd_data->type == TYPE_AFFT;
}

int fm_isfile(int fd) {
    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL || fd_data->type == TYPE_FREE)
        return -EBADF;

    return fd_data->type == TYPE_FILE;
}

int fm_fcntl(int fd, int cmd, int arg) {
    fd_data_t *fd_data = fm_fd_to_data(fd);
    if (fd_data == NULL || fd_data->type == TYPE_FREE)
        return -EBADF;

    switch (cmd) {
        case F_DUPFD:
            for (int i = fd; i < MAX_FD; i++) {
                fd_data_t *fd_data = fm_fd_to_data(i);
                if (fd_data->type != TYPE_FREE)
                    continue;
                if (fm_dup2(fd, i) < 0)
                    return -EBADF;
                return i;
            }
            return -EMFILE;
        case F_GETFD:
            return 0;
        case F_SETFD:
            return 0;
        case F_GETFL:
            return fd_data->flags;
        case F_SETFL:
            fd_data->flags = arg;
            return 0;
        case F_GETLK:
            return -0xFFFF;
        case F_SETLK:
            return -0xFFFF;
        case F_SETLKW:
            return -0xFFFF;
        default:
            return -0xFFFF;
    }
}

uint32_t fm_get_sid(int fd) {
    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL || fd_data->type == TYPE_FREE)
        return SID_NULL;

    return fd_data->sid;
}

const char *fm_get_path(int fd) {
    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL || fd_data->type != TYPE_DIR)
        return NULL;

    return fd_data->path;
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

int __init(void) {
    open_pipes = calloc(PIPE_MAX * sizeof(pipe_data_t));

    // open default fds
    return (
        fm_reopen(0, "/dev/kterm", O_RDONLY) < 0 ||
        fm_reopen(1, "/dev/kterm", O_WRONLY) < 0 ||
        fm_reopen(2, "/dev/kterm", O_WRONLY) < 0
    );
}

void *__profan_module_func[] = {
    (void *) 0xF3A3C4D4, // magic
    (void *) fm_close,
    (void *) fm_reopen,
    (void *) fm_pread,
    (void *) fm_pwrite,
    (void *) fm_lseek,
    (void *) fm_dup2,
    (void *) fm_dup,
    (void *) fm_pipe,
    (void *) fm_isafft,
    (void *) fm_isfile,
    (void *) fm_fcntl,
    (void *) fm_get_sid,
    (void *) fm_get_path,
    (void *) fm_declare_child
};
