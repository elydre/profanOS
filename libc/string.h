#ifndef STRINGS_H
#define STRINGS_H

#include "function.h"
#include "../drivers/screen.h"

#define strcpy_s(s1, s2) (void)(ARYLEN(s1) > strlen(s2) ? strcpy(s1, s2) : ckprint("Error: strcpy_s: target string is too small\n", c_red))

void int_to_ascii(int n, char str[]);
void hex_to_ascii(int n, char str[]);
int ascii_to_int(char str[]);
void reverse(char s[]);
int strlen(char s[]);
void backspace(char s[]);
void append(char s[], char n);
void strcpy(char s1[], char s2[]);
int strcmp(char s1[], char s2[]);
void str_start_split(char s[], char delim);
void str_end_split(char s[], char delim);

#endif
