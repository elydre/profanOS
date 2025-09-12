#ifndef RTL8168_H
#define RTL8168_H

extern int rtl8168_is_inited;

int rtl8168_send_packet(const void * p_data, uint16_t p_len);
int rtl8168_init(void);
void rtl8168_set_mac(uint8_t mac[6]);

#endif
