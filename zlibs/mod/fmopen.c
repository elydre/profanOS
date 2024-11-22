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

#define MAX_PIPE 256

enum {
    TYPE_FREE = 0,
    TYPE_FILE,
    TYPE_FCTF,
    TYPE_PPRD, // read pipe
    TYPE_PPWR  // write pipe
};

typedef struct {

} pipe_t;

typedef struct {
    uint8_t type;

    union {
        uint32_t  sid; // for file
        int     (*fctf)(int, void *, uint32_t, uint8_t);
        pipe_t   *pipe;
    };

    union {
        int fctf_id; // fcft only
        int offset;  // file
    };
} fd_data_t;

pipe_t *open_pipes;

int main(void) {
    open_pipes = calloc_ask(MAX_PIPE, sizeof(pipe_t));
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
    return NULL;
}

int fm_close(int fd) {
    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL || fd_data->type == TYPE_FREE)
        return -EBADF;

    if (fd_data->type == TYPE_FCTF)
        fd_data->fctf(fd_data->fctf_id, NULL, 0, FCTF_CLOSE);

    fd_data->type = TYPE_FREE;
    return 0;
}

int fm_reopen(int fd, char *abs_path, int flags) {
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

    fd_printf(2, "fd: %d, sid: %x, flags: %x\n", fd, sid, flags);

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
            if (fd_data->offset > (int) tmp)
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
            break;

        default:
            return -EBADF;
    }

    fd_data->offset += write_count;
    return write_count;
}

int fm_lseek(int fd, int offset, int whence) {
    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL)
        return -EBADF;

    if (fd_data->type != TYPE_FILE)
        return -ESPIPE;

    // TODO: lseek for pipes ?

    switch (whence) {
        case SEEK_SET:
            fd_data->offset = offset;
            break;
        case SEEK_CUR:
            fd_data->offset += offset;
            break;
        case SEEK_END:
            if (fd_data->type != TYPE_FILE)
                return -ESPIPE;
            fd_data->offset = syscall_fs_get_size(NULL, fd_data->sid) + offset;
            break;
        default:
            return -EINVAL;
    }

    if (fd_data->offset < 0)
        fd_data->offset = 0;

    return fd_data->offset;
}

int fm_tell(int fd) {
    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL)
        return -EBADF;

    if (fd_data->type != TYPE_FILE)
        return -ESPIPE;

    // TODO: tell for pipes ?

    return fd_data->offset;
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

        // TODO: dup for pipes

        default:
            return -EBADF;
    }

    return new_fd;
}

int fm_dup(int fd) {
    int new_fd; fm_get_free_fd(&new_fd);

    if (new_fd < 0)
        return -EMFILE;

    return fm_dup2(fd, new_fd);
}

int fm_pipe(int fd[2]) {
    return 0;
}

int fm_isfctf(int fd) {
    fd_data_t *fd_data = fm_fd_to_data(fd);

    if (fd_data == NULL || fd_data->type == TYPE_FREE)
        return -EBADF;
    
    return fd_data->type == TYPE_FCTF;
}
