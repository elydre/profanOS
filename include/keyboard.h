#ifndef KEYBOARD_H
#define KEYBOARD_H

char scancode_to_char(int scancode, int shift);
int get_last_scancode();

void init_keyboard();

#endif
