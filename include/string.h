#ifndef STRINGS_H
#define STRINGS_H

#include <function.h>
#include <iolib.h>

#define str_cpy_s(s1, s2) (void)(ARYLEN(s1) > str_len(s2) ? str_cpy(s1, s2) : fskprint("$Estr_cpy_s: $3target string is too small\n"))

void  int_to_ascii(int n, char str[]);
void  hex_to_ascii(int n, char str[]);
int   ascii_to_int(char str[]);
void  str_reverse(char s[]);
int   str_len(char s[]);
void  str_backspace(char s[]);
void  str_append(char s[], char n);
void  str_cpy(char s1[], char s2[]);
int   str_cmp(char s1[], char s2[]);
void  str_start_split(char s[], char delim);
void  str_end_split(char s[], char delim);
int   str_is_in(char str[], char thing);
int   str_count(char str[], char thing);
char* str_cat(char s1[], const char s2[]);
void str_delchar(char s[], char c);


#endif
