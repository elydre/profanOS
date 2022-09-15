#ifndef STRINGS_H
#define STRINGS_H

#include <function.h>
#include <iolib.h>

#define strcpy_s(s1, s2) (void)(ARYLEN(s1) > strlen(s2) ? strcpy(s1, s2) : fskprint("$Estrcpy_s: $3target string is too small\n"))

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
int in_string(char str[], char thing);
int count_string(char str[], char thing);

#endif
