/*****************************************************************************\
|   === dhcp.h : 2025 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef DHCP_H
#define DHCP_H

#include <net/ethernet.h>

// En-tête DHCP (240 octets sans options)
struct dhcp_header {
    uint8_t  op;       // 1 = Request, 2 = Reply
    uint8_t  htype;    // 1 = Ethernet
    uint8_t  hlen;     // Taille adresse MAC (6)
    uint8_t  hops;     // Nombre de relais (0)
    uint32_t xid;      // Identifiant transaction
    uint16_t secs;     // Secondes écoulées
    uint16_t flags;    // Flags (0x8000 pour broadcast)
    uint32_t ciaddr;   // Client IP (0.0.0.0)
    uint32_t yiaddr;   // IP assignée par le serveur
    uint32_t siaddr;   // IP du serveur
    uint32_t giaddr;   // IP du relais
    uint8_t  chaddr[16]; // Adresse MAC du client (6 octets) + padding
    uint8_t  sname[64]; // Nom du serveur (optionnel)
    uint8_t  file[128]; // Nom du fichier de boot (optionnel)
    uint32_t magic_cookie; // 0x63825363 (Magic DHCP)
    uint8_t  options[]; // Options DHCP
} __attribute__((packed));

typedef struct dhcp_header dhcp_header_t;

typedef struct {
    uint8_t types[50];
    uint8_t lens[50];
    uint8_t *datas[50];
} dhcp_options_t;

void build_dhcp_packet(uint8_t *buffer, uint8_t *mac);

#endif
