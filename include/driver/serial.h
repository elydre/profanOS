#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_PORT_A 0x3F8
#define SERIAL_PORT_B 0x2F8

void serial_init();
void serial_string(int device, char out[]);
void serial_debug(char source[], char message[]);


#endif
