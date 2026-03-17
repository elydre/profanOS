/*****************************************************************************\
|   === rtl8168.h : 2026 ===                                                  |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef RTL8168_H
#define RTL8168_H

extern int rtl8168_is_inited;

int rtl8168_send_packet(const void * p_data, uint16_t p_len);
int rtl8168_init(void);
void rtl8168_set_mac(uint8_t mac[6]);

#endif
