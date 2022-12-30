#ifndef MOUSE_H
#define MOUSE_H

#include <type.h>
#include <cpu/isr.h>

void mouse_handler(registers_t *a_r);
void mouse_write(byte a_write);
byte mouse_read();
void mouse_install();
int mouse_get_x();
int mouse_get_y();
bool mouse_get_button(int button);
void mouse_set_x(int x);
void mouse_set_y(int y);
void mouse_set_button(int button, bool value);
void mouse_reset();

#endif