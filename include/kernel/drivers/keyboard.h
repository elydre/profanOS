/****** This file is part of profanOS **************************\
|   == keyboard.h ==                                 .pi0iq.    |
|                                                   d"  . `'b   |
|   Kernel Keyboard driver header                   q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KB_A 16
#define KB_Z 17
#define KB_E 18
#define KB_R 19
#define KB_T 20
#define KB_Y 21
#define KB_U 22
#define KB_I 23
#define KB_O 24
#define KB_P 25
#define KB_Q 30
#define KB_S 31
#define KB_D 32
#define KB_F 33
#define KB_G 34
#define KB_H 35
#define KB_J 36
#define KB_K 37
#define KB_L 38
#define KB_M 39
#define KB_N 49
#define KB_W 44
#define KB_X 45
#define KB_C 46
#define KB_V 47
#define KB_B 48

#define KB_ESC 1
#define KB_ALT 56
#define KB_CTRL 29
#define KB_SHIFT 42
#define KB_MAJ 58
#define KB_TAB 15

#define TAB_SIZE 4

#define KB_released_value 128
#define KB_released(key) (key + KB_released_value)

char kb_scancode_to_char(int scancode, int shift);

int  kb_get_scancode(void);
int  kb_get_scfh(void);

int keyboard_init(void);

#endif
