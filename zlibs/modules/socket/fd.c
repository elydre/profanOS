/*****************************************************************************\
|   === fd.c : 2026 ===                                                       |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <modules/socket.h>
#include <kernel/process.h>
#include <minilib.h>
#include <errno.h>

socket_t *sockets = NULL;
int sockets_len = 0;

int last_id = 0;
extern int socket_pid;

int socket_socket(int domain, int type_, int protocol) {
    uint32_t type = domain | (type_ << 8) | (protocol << 16);
    int res = -EINVAL;
    protocol_t *prot = socket_find_protocol(type);
    if (!prot || !prot->init)
        return -EINVAL;
    fd_data_t *fd_data = fm_get_free_fd(&res);
    if (fd_data == NULL)
        return -EMFILE;
    sockets = realloc(sockets, sizeof(socket_t) * (sockets_len + 1));
    sockets[sockets_len] = (socket_t){0};
    sockets[sockets_len].type = type;
    sockets[sockets_len].id = last_id;
    sockets[sockets_len].ref_count = 1;
    sockets[sockets_len].do_remove = 0;

    int err = prot->init(&sockets[sockets_len]);
    if (err)
        return err;

    fd_data->sock_id = last_id;
    fd_data->type = TYPE_SOCK;

    sockets_len++;
    last_id++;
    if (process_info(socket_pid, PROC_INFO_STATE, NULL) == PROC_STATE_SLP)
        process_wakeup(socket_pid, 0); // TODO: asqel check why the condition is needed
    return res;
}

socket_t *socket_find_id(int id) {
    for (int i = 0; i < sockets_len; i++) {
        if (sockets[i].id == id)
            return &sockets[i];
    }
    return NULL;
}

socket_t *socket_find_fd(int fd) {
    fd_data_t *data = fm_fd_to_data(fd);
    if (!data || data->type != TYPE_SOCK)
        return NULL;
    return socket_find_id(data->sock_id);
}
