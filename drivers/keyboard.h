#ifndef KEYBOARD_H
#define KEYBOARD_H

char scancode_to_char(int scancode);
void scancode_to_name(int scancode, char name[]);

void init_keyboard();

#endif
