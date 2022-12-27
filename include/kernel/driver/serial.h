#ifndef SERIAL_H
#define SERIAL_H

#include <type.h>

#define SERIAL_PORT_A 0x3F8
#define SERIAL_PORT_B 0x2F8


int serial_init();
void serial_debug(char source[], char message[]);
void serial_print(int device, char out[]);

#endif
