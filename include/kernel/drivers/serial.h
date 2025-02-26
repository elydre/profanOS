#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_PORT_A 0x3F8
#define SERIAL_PORT_B 0x2F8

int serial_init(void);

int serial_write(uint32_t port, void *buffer, uint32_t len);
int serial_read(uint32_t port, void *buffer, uint32_t len);

#endif
