#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <i_iolib.h>
#include <panda.h>

#define DEVIO_LIB_C
#include <filesys.h>

// modes definition
#define MODE_WRITE 0
#define MODE_READD 1

void init_devio();
void init_lcbuffer();

typedef struct {
    sid_t link;
    int pid;
    uint32_t offset;
    sid_t redirection;
} link_history_t;

link_history_t *link_history = NULL;

#define LINK_HISTORY_SIZE 10

int main(void) {
    init_devio();
    return 0;
}

int devio_set_redirection(sid_t link, char *redirection, int pid) {
    if (!fu_is_link(link)) {
        printf("[DEVIO] set_redirection: given sid is not a link\n");
        return 1;
    }

    if (pid < 0)
        pid = c_process_get_pid();

    char **paths;
    int *pids;

    int link_count = fu_link_get_all(link, &pids, &paths);

    if (link_count < 0) {
        printf("[DEVIO] set_redirection: error while getting link\n");
        return 1;
    }

    if (link_count == 0) {
        if (pid != 0)
            fu_link_add_path(link, 0, redirection);
        fu_link_add_path(link, pid, redirection);
        return 0;
    }

    // check if the pid is in the array
    int pid_in, default_in;
    pid_in = default_in = 0;
    for (int i = 0; i < link_count; i++) {
        if (pids[i] == pid) {
            pid_in = 1;
        }
        if (pids[i] == 0) {
            default_in = 1;
        }
    }

    if (pid_in)
        fu_link_remove_path(link, pid);

    if (!default_in)
        fu_link_add_path(link, 0, redirection);
    fu_link_add_path(link, pid, redirection);

    // free the arrays
    for (int i = 0; i < link_count; i++) {
        free(paths[i]);
    }
    free(paths);
    free(pids);

    // remove the redirection from the history
    for (int i = 0; i < LINK_HISTORY_SIZE; i++) {
        if (IS_SAME_SID(link_history[i].link, link)) {
            link_history[i].link = NULL_SID;
        }
    }

    return 0;
}

int devio_file_rw_from(sid_t sid, void *buffer, uint32_t offset, uint32_t size, uint8_t is_write, int pid) {
    int ret;

    if (IS_NULL_SID(sid)) {
        return -1;
    }

    if (size == 0) {
        return 0;
    }

    // function call though filesystem
    if (fu_is_fctf(sid)) {
        if (is_write)
            return fu_fctf_write(sid, buffer, offset, size);
        return fu_fctf_read(sid, buffer, offset, size);
    }

    // classic file
    if (fu_is_file(sid)) {
        if (is_write) {
            if (size + offset > fu_get_file_size(sid))
                if (fu_set_file_size(sid, size + offset)) return 0;
            return fu_file_write(sid, buffer, offset, size) ? 0 : size;
        }
        if (size + offset > fu_get_file_size(sid))
            size = fu_get_file_size(sid) - offset;
        if (size == 0) return 0;
        return fu_file_read(sid, buffer, offset, size) ? 0 : size;
    }

    if (!fu_is_link(sid)) {
        printf("no method to write in sid\n");// d%ds%d\n", sid.device, sid.sector);
        return -1;
    }

    // text link
    if (pid < 0)
        pid = c_process_get_pid();

    // check if the pid is in the history
    for (int i = 0; i < LINK_HISTORY_SIZE; i++) {
        if (IS_SAME_SID(link_history[i].link, sid) && link_history[i].pid == pid) {
            ret = devio_file_rw_from(link_history[i].redirection, buffer, link_history[i].offset, size, is_write, pid);
            link_history[i].offset += ret;
            return ret;
        }
    }

    char **paths;
    int *pids;

    int link_count = fu_link_get_all(sid, &pids, &paths);

    if (link_count < 0) {
        c_kprint("[DEVIO] file_rw: error while getting link\n");
        return -1;
    }

    if (link_count == 0) {
        c_kprint("[DEVIO] file_rw: empty link\n");
        return -1;
    }

    char *path = NULL;
    int tmp;

    char fe[256];

    // check if the pid is in the array
    // else check for the ppids
    tmp = pid;
    while (!path) {
        for (int i = 0; i < link_count; i++) {
            if (pids[i] == tmp) {
                path = strdup(paths[i]);
                break;
            }
        }
        if (tmp == 0) break;

        tmp = c_process_get_ppid(tmp);
    }

    // free the arrays
    for (int i = 0; i < link_count; i++) {
        free(paths[i]);
    }
    free(paths);
    free(pids);

    if (!path) {
        printf("[DEVIO] file_rw: no path found\n");
        return -1;
    }

    sid_t new_sid = fu_path_to_sid(ROOT_SID, path);
    free(path);

    if (IS_NULL_SID(new_sid)) {
        printf("[DEVIO] file_rw: error while getting sid\n");
        return -1;
    }

    int index = 0;
    for (index = 0; index < LINK_HISTORY_SIZE; index++) {
        if (IS_NULL_SID(link_history[index].link)) {
            link_history[index].link = sid;
            link_history[index].pid = pid;
            link_history[index].redirection = new_sid;
            break;
        }
        if (index == LINK_HISTORY_SIZE - 1) {
            // move all the history
            for (int j = LINK_HISTORY_SIZE - 2; j >= 0; j--) {
                link_history[j + 1] = link_history[j];
            }
            link_history[0].link = sid;
            link_history[0].pid = pid;
            link_history[0].redirection = new_sid;
            link_history[0].offset = 0;
            index = 0;
            break;
        }
    }

    ret = devio_file_rw_from(new_sid, buffer, 0, size, is_write, pid);
    link_history[index].offset = ret;
    return ret;
}

int devnull_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_READD) {
        memset(buffer, 0, size);
        return size;
    }
    return 0;
}

int devrand_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode != MODE_READD) {
        return 0;
    }

    for (uint32_t i = 0; i < size; i++) {
        ((uint8_t *) buffer)[i] = (uint8_t) rand();
    }

    return size;
}

int devzebra_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_WRITE) {
        c_kcprint((char *) buffer, c_dgreen);
        return size;
    }

    return 0;
}

int devparrot_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_WRITE) {
        color_print((char *) buffer);
        return size;
    }

    return 0;
}

int devpanda_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    static char color = c_white;
    if (mode == MODE_WRITE) {
        color = panda_color_print((char *) buffer, color, size);
        return size;
    } else if (mode == MODE_READD) {
        if (size < 2 * sizeof(uint32_t)) {
            return 0;
        }
        panda_get_cursor((uint32_t *) buffer, (uint32_t *) buffer + 1);
        return 2 * sizeof(uint32_t);
    }

    return 0;
}

int devserial_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_WRITE) {
        c_serial_print(SERIAL_PORT_A, (char *) buffer);
        return size;
    }

    return 0;
}

int devkb_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_READD) {
        return open_input(buffer, size);
    }

    return 0;
}

void init_devio(void) {
    fu_fctf_create(0, "/dev/null",   devnull_rw);
    fu_fctf_create(0, "/dev/random", devrand_rw);

    fu_fctf_create(0, "/dev/zebra",  devzebra_rw);
    fu_fctf_create(0, "/dev/parrot", devparrot_rw);
    fu_fctf_create(0, "/dev/panda",  devpanda_rw);
    fu_fctf_create(0, "/dev/serial", devserial_rw);
    fu_fctf_create(0, "/dev/kb",     devkb_rw);

    link_history = calloc(LINK_HISTORY_SIZE, sizeof(link_history_t));

    devio_set_redirection(fu_link_create(0, "/dev/stdin"),  "/dev/kb", 0);
    devio_set_redirection(fu_link_create(0, "/dev/stdout"), "/dev/parrot", 0);
    devio_set_redirection(fu_link_create(0, "/dev/stderr"), "/dev/parrot", 0);
}
