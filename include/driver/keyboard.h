#ifndef KEYBOARD_H
#define KEYBOARD_H

char kb_scancode_to_char(int scancode, int shift);
int kb_get_scancode();

void keyboard_init();

#endif
