/*****************************************************************************\
|   === ip-get.c : 2026 ===                                                   |
|                                                                             |
|    DHCP IP retrieval utility for profanOS                        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <profan/syscall.h>
#include <modules/eth.h>
#include <profan/carp.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    uint8_t  dest_mac[6];
    uint8_t  src_mac[6];
    uint16_t ether_type;
} __attribute__((packed)) ethernet_header_t;

typedef struct {
    uint8_t  version_ihl;
    uint8_t  tos;
    uint16_t total_length;
    uint16_t id;
    uint16_t flags_offset;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dest_ip;
} __attribute__((packed)) ip_header_t;

typedef struct {
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed)) udp_header_t;

typedef struct {
    uint8_t  op;
    uint8_t  htype;
    uint8_t  hlen;
    uint8_t  hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint32_t ciaddr;
    uint32_t yiaddr;
    uint32_t siaddr;
    uint32_t giaddr;
    uint8_t  chaddr[16];
    uint8_t  sname[64];
    uint8_t  file[128];
    uint32_t magic_cookie;
} __attribute__((packed)) dhcp_packet_no_opt_t;

uint32_t last_xid = 0;
uint32_t offered_ip = 0;
uint32_t dhcp_server_ip = 0;

eth_info_t g_info;
uint32_t g_eth_id;

/***************************************
 *                                    *
 *              REQUEST               *
 *                                    *
***************************************/

void send_dhcp_request(uint8_t *mac) {
    int total_size = sizeof(ethernet_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t)
        + sizeof(dhcp_packet_no_opt_t) + 16;
    uint8_t *packet = malloc(total_size);
    if (!packet)
        return;

    ethernet_header_t *eth = (ethernet_header_t *)packet;
    ip_header_t *ip = (ip_header_t *)(packet + sizeof(ethernet_header_t));
    udp_header_t *udp = (udp_header_t *)(packet + sizeof(ethernet_header_t) + sizeof(ip_header_t));
    dhcp_packet_no_opt_t *dhcp = (dhcp_packet_no_opt_t *)(packet + sizeof(ethernet_header_t) +
                sizeof(ip_header_t) + sizeof(udp_header_t));
    uint8_t *opt = (uint8_t *)(dhcp + 1);

    memcpy(eth->dest_mac, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
    memcpy(eth->src_mac, mac, 6);
    eth->ether_type = htons(0x0800);

    ip->version_ihl = 0x45;
    ip->tos = 0;
    ip->id = htons(0);
    ip->flags_offset = htons(0x4000);
    ip->ttl = 64;
    ip->protocol = 17; // UDP
    ip->checksum = 0;
    ip->src_ip = htonl(0x00000000); // 0.0.0.0
    ip->dest_ip = htonl(0xFFFFFFFF); // 255.255.255.255
    ip->total_length = htons(sizeof(ip_header_t) + sizeof(udp_header_t) + sizeof(dhcp_packet_no_opt_t) + 16);

    udp->src_port = htons(68);
    udp->dest_port = htons(67);
    udp->length = htons(sizeof(udp_header_t) + sizeof(dhcp_packet_no_opt_t) + 16);
    udp->checksum = 0;

    memset(dhcp, 0, sizeof(dhcp_packet_no_opt_t));
    dhcp->op = 1; // BOOTREQUEST
    dhcp->htype = 1; // Ethernet
    dhcp->hlen = 6;
    dhcp->hops = 0;
    dhcp->xid = last_xid;
    dhcp->secs = htons(1);
    dhcp->flags = htons(0x8000);
    dhcp->ciaddr = 0;
    dhcp->yiaddr = 0;
    dhcp->siaddr = dhcp_server_ip;
    dhcp->giaddr = 0;
    memcpy(dhcp->chaddr, mac, 6);
    memset(dhcp->sname, 0, sizeof(dhcp->sname));
    memset(dhcp->file, 0, sizeof(dhcp->file));
    dhcp->magic_cookie = htonl(0x63825363);
    *(opt++) = 53; // DHCP Message Type
    *(opt++) = 1; // Length
    *(opt++) = 3; // DHCPREQUEST
    *(opt++) = 50; // Requested IP Address
    *(opt++) = 4; // Length
    memcpy(opt, &offered_ip, 4);
    opt += 4;
    *(opt++) = 54;
    *(opt++) = 4; // Length
    memcpy(opt, &dhcp_server_ip, 4);
    opt += 4;
    *(opt++) = 0xFF;
    // checksum

    // === Checksum IP ===
    ip->checksum = 0;
    uint32_t sum = 0;
    uint16_t ip_data[sizeof(ip_header_t)/2];
    memcpy(ip_data, ip, sizeof(ip_header_t));
    for (size_t i = 0; i < sizeof(ip_header_t)/2; ++i)
        sum += ntohs(ip_data[i]);
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum += (sum >> 16);
    ip->checksum = htons(~sum);

    // === Checksum UDP ===
    sum = 0;
    // Pseudo-header
    sum += (ip->src_ip >> 16) & 0xFFFF;
    sum += ip->src_ip & 0xFFFF;
    sum += (ip->dest_ip >> 16) & 0xFFFF;
    sum += ip->dest_ip & 0xFFFF;
    sum += ip->protocol;
    sum += ntohs(udp->length);

    int dhcp_size = sizeof(dhcp_packet_no_opt_t) + (opt - (uint8_t *)(dhcp + 1));
    // UDP + DHCP
    uint8_t *udp_data = (uint8_t *)udp;
    for (size_t i = 0; i < sizeof(udp_header_t) + dhcp_size; i += 2) {
        uint16_t word = udp_data[i] << 8;
        if (i + 1 < sizeof(udp_header_t) + dhcp_size)
            word |= udp_data[i + 1];
        sum += word;
    }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum += (sum >> 16);
    udp->checksum = htons(~sum);

    modeth_send(packet, total_size);
}

/***************************************
 *                                    *
 *            ACKNOWLEDGE             *
 *                                    *
***************************************/

int receive_ack(uint8_t *mac) {
    (void)mac;
    int size = modeth_is_ready(g_eth_id);
    if (size < 0) {
        return 1;
    }
    int packet_size = size;
    uint8_t *packet = malloc(packet_size);
    if (!packet) {
        return 1;
    }
    modeth_recv(g_eth_id, packet);
    ethernet_header_t *eth = (ethernet_header_t *)packet;
    if (eth->ether_type != htons(0x0800)) {
        free(packet);
        return 1;
    }

    ip_header_t *ip = (ip_header_t *)(packet + sizeof(ethernet_header_t));
    if (ip->protocol != 17) {
        free(packet);
        return 1;
    }

    udp_header_t *udp = (udp_header_t *)((uint8_t *)ip + sizeof(ip_header_t));
    if (ntohs(udp->src_port) != 67 || ntohs(udp->dest_port) != 68) {
        free(packet);
        return 1;
    }

    dhcp_packet_no_opt_t *dhcp = (dhcp_packet_no_opt_t *)((uint8_t *)udp + sizeof(udp_header_t));
    if (dhcp->op != 2 || dhcp->xid != last_xid) {
        free(packet);
        return 1;
    }

    uint8_t *opt = (uint8_t *)(dhcp + 1);
    if (opt >= packet + packet_size) {
        free(packet);
        return 1;
    }
    int opt_len = packet_size - (opt - packet);
    uint32_t router_mask = 0;
    int is_ack = 0;
    int i = 0;

    while (i < opt_len && opt[i] != 0xff) {
        if (opt[i] == 0) {
            i++;
            continue;
        }
        if (i + 2 < opt_len) {
            if (opt[i] == 53 && opt[i + 1] == 1 && opt[i + 2] == 5)
                is_ack = 1;
        }
        if (opt[i] == 0x01 && i + 5 < opt_len && opt[i + 1] == 4)
            router_mask = *(uint32_t *)(&opt[i + 2]);
        if (i + 1 < opt_len)
            i += opt[i + 1] + 2;
        else
            break;
    }
    free(packet);
    if (router_mask == 0 || is_ack == 0)
        return 1;
    g_info.net_mask = router_mask;
    return 0;
}

/***************************************
 *                                    *
 *              DISCOVER              *
 *                                    *
***************************************/

void send_dhcp_discover(uint8_t *mac) {
    uint8_t packet[512];
    memset(packet, 0, sizeof(packet));

    // === Pointeurs vers les structures ===
    ethernet_header_t *eth = (ethernet_header_t *)packet;
    ip_header_t *ip = (ip_header_t *)(packet + sizeof(ethernet_header_t));
    udp_header_t *udp = (udp_header_t *)((uint8_t *)ip + sizeof(ip_header_t));
    dhcp_packet_no_opt_t *dhcp = (dhcp_packet_no_opt_t *)((uint8_t *)udp + sizeof(udp_header_t));
    uint8_t *options = (uint8_t *)(dhcp + 1);

    // === Remplissage Ethernet ===
    memset(eth->dest_mac, 0xFF, 6);  // Broadcast
    memcpy(eth->src_mac, mac, 6);
    eth->ether_type = htons(0x0800); // IPv4

    // === Remplissage IP ===
    ip->version_ihl = 0x45;  // IPv4, IHL = 5
    ip->tos = 0;
    ip->id = htons(0);
    ip->flags_offset = htons(0x4000); // Don't fragment
    ip->ttl = 64;
    ip->protocol = 17; // UDP
    ip->checksum = 0;
    ip->src_ip = htonl(0x00000000); // 0.0.0.0
    ip->dest_ip = htonl(0xFFFFFFFF); // 255.255.255.255

    // === Remplissage UDP ===
    udp->src_port = htons(68);
    udp->dest_port = htons(67);
    udp->checksum = 0; // Calculé plus tard

    // === Remplissage DHCP sans options ===
    memset(dhcp, 0, sizeof(dhcp_packet_no_opt_t));
    dhcp->op = 1; // BOOTREQUEST
    dhcp->htype = 1; // Ethernet
    dhcp->hlen = 6;
    dhcp->xid = htonl(modeth_get_transaction());
    last_xid = dhcp->xid;
    dhcp->flags = htons(0x8000); // Broadcast flag
    memcpy(dhcp->chaddr, mac, 6);
    dhcp->magic_cookie = htonl(0x63825363);

    // === DHCP Options ===
    uint8_t *opt = options;
    *opt++ = 53; *opt++ = 1; *opt++ = 1; // DHCPDISCOVER
    *opt++ = 55; *opt++ = 3; *opt++ = 1; *opt++ = 3; *opt++ = 6; // Parameter request
    *opt++ = 255; // End option

    size_t dhcp_size = sizeof(dhcp_packet_no_opt_t) + (opt - options);
    size_t udp_len = sizeof(udp_header_t) + dhcp_size;
    size_t ip_len = sizeof(ip_header_t) + udp_len;
    size_t total_len = sizeof(ethernet_header_t) + ip_len;

    // === Longueurs ===
    ip->total_length = htons(ip_len);
    udp->length = htons(udp_len);

    // === Checksum IP ===
    ip->checksum = 0;
    uint32_t sum = 0;
    // uint16_t *ip_data = (uint16_t *)ip;
    // for (size_t i = 0; i < sizeof(ip_header_t)/2; ++i)
    //     sum += ntohs(ip_data[i]);
    uint16_t ip_data_buff[sizeof(ip_header_t) / 2];
    memcpy(ip_data_buff, ip, sizeof(ip_header_t));
    for (size_t i = 0; i < sizeof(ip_data_buff) / 2; i++) {
        sum += ntohs(ip_data_buff[i]);
    }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum += (sum >> 16);
    ip->checksum = htons(~sum);

    // === Checksum UDP ===
    sum = 0;
    // Pseudo-header
    sum += (ip->src_ip >> 16) & 0xFFFF;
    sum += ip->src_ip & 0xFFFF;
    sum += (ip->dest_ip >> 16) & 0xFFFF;
    sum += ip->dest_ip & 0xFFFF;
    sum += ip->protocol;
    sum += ntohs(udp->length);

    // UDP + DHCP
    uint8_t *udp_data = (uint8_t *)udp;
    for (size_t i = 0; i < udp_len; i += 2) {
        uint16_t word = udp_data[i] << 8;
        if (i + 1 < udp_len)
            word |= udp_data[i + 1];
        sum += word;
    }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum += (sum >> 16);
    udp->checksum = htons(~sum);

    // === Envoi ===
    modeth_send(packet, total_len);
}

/***************************************
 *                                    *
 *               OFFER                *
 *                                    *
***************************************/

int receive_offer(uint8_t *mac) {
    (void)mac;
    int size = modeth_is_ready(g_eth_id);
    if (size < 0) {
        return 1;
    }
    int packet_size = size;
    uint8_t *packet = malloc(packet_size);
    if (!packet) {
        return 1;
    }
    modeth_recv(g_eth_id, packet);
    ethernet_header_t *eth = (ethernet_header_t *)packet;
    if (eth->ether_type != htons(0x0800)) {
        free(packet);
        return 1;
    }

    ip_header_t *ip = (ip_header_t *)(packet + sizeof(ethernet_header_t));
    if (ip->protocol != 17) {
        free(packet);
        return 1;
    }

    udp_header_t *udp = (udp_header_t *)((uint8_t *)ip + sizeof(ip_header_t));
    if (ntohs(udp->src_port) != 67 || ntohs(udp->dest_port) != 68) {
        free(packet);
        return 1;
    }

    dhcp_packet_no_opt_t *dhcp = (dhcp_packet_no_opt_t *)((uint8_t *)udp + sizeof(udp_header_t));
    if (dhcp->op != 2 || dhcp->xid != last_xid) {
        free(packet);
        return 1;
    }

    uint8_t *opt = (uint8_t *)(dhcp + 1);
    if (opt >= packet + packet_size) {
        free(packet);
        return 1;
    }
    int opt_len = packet_size - (opt - packet);
    for (int i = 0; i < opt_len && opt[i] != 0xff; ) {
        if (opt[i] == 0) {
            i++;
            continue;
        }
        if (i + 2 < opt_len) {
            if (opt[i] == 53 && opt[i + 1] == 1 && opt[i + 2] == 2) {
                offered_ip = dhcp->yiaddr;
                dhcp_server_ip = dhcp->siaddr;
                free(packet);
                memcpy(g_info.router_mac, mac, 6);
                return 0;
            }
        }
        if (i + 1 < opt_len)
            i += opt[i + 1] + 2;
        else
            break;
    }
    free(packet);
    return 1;
}

/***************************************
 *                                    *
 *                MAIN                *
 *                                    *
***************************************/

int retrieve_ip(uint8_t *mac) {
    srand(syscall_timer_get_ms());

    send_dhcp_discover(mac);

    uint32_t now = syscall_timer_get_ms();
    int fail = 1;
    while (syscall_timer_get_ms() - now < 2000) {
        fail = receive_offer(mac);
        if (fail == 0)
            break;
        usleep(10000);
    }
    if (fail)
        return 1;
    send_dhcp_request(mac);
    now = syscall_timer_get_ms();
    fail = 1;
    while (syscall_timer_get_ms() - now < 2000) {
        fail = receive_ack(mac);
        if (fail == 0)
            break;
        usleep(10000);
    }
    return fail;
}

void print_ip(uint32_t ip) {
    if (carp_isset('q'))
        return;
    printf("%d.%d.%d.%d\n", ip & 0xff, 0xff & (ip >> 8), 0xff & (ip >> 16), ip >> 24);
}

int main(int argc, char **argv) {
    carp_init("[-fq]", 0);

    carp_register('f', CARP_STANDARD, "force getting a new IP address");
    carp_register('q', CARP_STANDARD, "quiet mode, don't print the IP address");

    if (carp_parse(argc, argv))
        return 1;

    int quiet = carp_isset('q');

    g_eth_id = modeth_start();
    if (g_eth_id == 0) {
        if (!quiet)
            fprintf(stderr, "ip-get: error: could not open connection with profan net\n");
        return 1;
    }

    modeth_get_info(g_eth_id, &g_info);
    if (g_info.ip != 0 && !carp_isset('f')) {
        print_ip(g_info.ip);
        modeth_end(g_eth_id);
        return 0;
    }

    if (!memcmp(g_info.mac, "\0\0\0\0\0\0", 6)) {
        if (!quiet)
            fprintf(stderr, "ip-get: error: no ethernet device found\n");
        modeth_end(g_eth_id);
        return 1;
    }

    int res = retrieve_ip(g_info.mac);
    if (res == 0) {
        memcpy(&g_info.ip, &offered_ip, 4);
        modeth_set_info(g_eth_id, &g_info);
        print_ip(offered_ip);
    }

    modeth_end(g_eth_id);
    return res;
}
