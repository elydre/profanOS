/*****************************************************************************\
|   === on_recv.c : 2026 ===                                                  |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/


#include <minilib.h>

#include "tcp.h"


void socket_on_recv_tcp(uint32_t src_ip, uint32_t dest_ip, uint8_t *data, int data_len) {
    tcp_packet_t packet;

    if (data_len < 20)
        return ;

    packet.ip_src = src_ip;
    packet.ip_dest = dest_ip;
    
    packet.port_src  = data[0] << 8;
    packet.port_src |= data[1];

    packet.port_dest  = data[2] << 8;
    packet.port_dest |= data[3];

    packet.seq  = (uint32_t)data[4] << 24;
    packet.seq |= (uint32_t)data[5] << 16;
    packet.seq |= (uint32_t)data[6] << 8;
    packet.seq |= (uint32_t)data[7];

    packet.ack  = (uint32_t)data[8] << 24;
    packet.ack |= (uint32_t)data[9] << 16;
    packet.ack |= (uint32_t)data[10] << 8;
    packet.ack |= (uint32_t)data[11];

    packet.data_offset = data[12] >> 4;

    packet.flags = data[13];

    packet.window  = data[14] << 8;
    packet.window |= data[15];

    packet.checksum  = data[16] << 8;
    packet.checksum |= data[17];

    packet.urgent_ptr  = data[18] << 8;
    packet.urgent_ptr |= data[19];

    packet.option = packet.data_offset > 5 ? &data[20] : NULL;
    packet.data = &data[packet.data_offset * 4];
    packet.data_len = data_len - packet.data_offset * 4;
    if (!packet.data_len)
        packet.data = NULL;

    // !TODO: check checksum check

    tcp_t *server_sock = NULL;
    tcp_t *client_sock = NULL;
    
    for (int i = 0; i < sockets_len; i++) {
        if (sockets[i].type != SOCKET_TCP)
            continue;
        tcp_t *sock = sockets[i].data;

        if (sock->local_port != packet.port_dest)
            continue;
    
        if (sock->local_ip != 0 && sock->local_ip != packet.ip_dest)
            continue;

        if (sock->remote_ip != 0 && sock->remote_ip != packet.ip_src)
            continue;

        if (sock->remote_port == 0)
            server_sock = sock; // server
       
        if (sock->remote_port == packet.port_src)
            client_sock = sock; // client
        
    }


    if (client_sock)
        udpate_sock_tcp(client_sock, &packet);
    else if (server_sock)
        update_sock_tcp(server_sock, &packet);
    else
        return ; // TODO: send RST
}
