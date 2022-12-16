#ifndef STRINGS_H
#define STRINGS_H

#include <function.h>
#include <system.h>

#define str_cpy_s(s1, s2) (void)(ARYLEN(s1) > str_len(s2) ? str_cpy(s1, s2) : sys_error("Destination buffer too small"))

void int_to_ascii(int n, char str[]);
void hex_to_ascii(int n, char str[]);
void double_to_ascii(double n, char str[]);
int  ascii_to_int(char str[]);
void str_reverse(char s[]);
int  str_len(char s[]);
void str_backspace(char s[]);
void str_append(char s[], char n);
void str_cpy(char s1[], char s2[]);
int  str_cmp(char s1[], char s2[]);
int  str_ncmp(char s1[], char s2[], int n);
void str_start_split(char s[], char delim);
void str_end_split(char s[], char delim);
int  str_is_in(char str[], char thing);
int  str_count(char str[], char thing);
void str_cat(char s1[], char s2[]);
void str_delchar(char s[], char c);
int  str_in_str(char s1[], char s2[]);

#endif
