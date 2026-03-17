/*****************************************************************************\
|   === mod.c : 2026 ===                                                      |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <modules/socket.h>

void *__module_func_array[] = {
    (void *)0xF3A3C4D4,
    socket_socket,
    socket_bind,
    socket_connect,
    socket_sendto,
    socket_recvfrom,
    socket_close_fd,
    socket_get_rw,
};
