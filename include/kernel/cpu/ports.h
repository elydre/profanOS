#ifndef PORTS_H
#define PORTS_H

#include <ktype.h>

uint8_t  port_byte_in(uint16_t port);
void     port_byte_out(uint16_t port, uint8_t data);

uint16_t port_word_in(uint16_t port);
void     port_word_out(uint16_t port, uint16_t data);

uint32_t port_long_in(uint32_t port);
void     port_long_out(uint32_t port, uint32_t value);

void outportb(uint16_t port, uint8_t val);
uint8_t inportb(uint16_t port);
uint16_t inports(uint16_t _port);
void outports(uint16_t _port, uint16_t _data);
uint32_t inportl(uint16_t _port);
void outportl(uint16_t _port, uint32_t _data);

#endif
