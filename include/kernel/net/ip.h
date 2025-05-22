/*****************************************************************************\
|   === ip.h : 2025 ===                                                       |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef IP_H
#define IP_H

// En-tête IP (20 octets minimum)
struct ip_header {
    uint8_t version_ihl;
    uint8_t  tos;              // Type of Service
    uint16_t total_length;     // Taille totale du paquet
    uint16_t id;               // ID du paquet
    uint16_t flags_fragment;   // Flags et offset de fragmentation
    uint8_t  ttl;              // Time to Live
    uint8_t  protocol;         // UDP = 17
    uint16_t checksum;         // Checksum IP
    uint32_t src_ip;           // IP source (0.0.0.0)
    uint32_t dst_ip;           // IP destination (255.255.255.255)
} __attribute__((packed));

typedef struct ip_header ip_header_t;

uint16_t ip_checksum(void *vdata, uint32_t length);

#endif
