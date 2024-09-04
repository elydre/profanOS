/*****************************************************************************\
|   === keyboard.h : 2024 ===                                                 |
|                                                                             |
|    Kernel Keyboard driver header                                 .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef KEYBOARD_H
#define KEYBOARD_H

int  keyboard_init(void);

char kb_sc_to_char(int scancode, int shift);
int  kb_get_scfh(void);

#endif
