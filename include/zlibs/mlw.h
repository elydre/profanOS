#ifndef MLW_H
#define MLW_H

#include <profan/syscall.h>
#include <profan/net.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

extern eth_info_t *mlw_eth_info;
extern uint32_t mlx_eth_id;

int mlw_ip_to_mac(uint32_t ip, uint8_t *mac_res);
int mlw_send_ip(uint8_t protocol, uint32_t dest_ip, const uint8_t *data, int len);

int mlw_send_ethernet(uint16_t type_lit, uint8_t *mac, uint8_t *data, int len);
int mlw_tcp_send(uint16_t src_port,
				uint32_t dest_ip, uint32_t dest_port, uint32_t seq,
				uint32_t ack, uint8_t flags, uint8_t *data, int len);

void mlw_init();

#endif
