#ifndef MINILIB_H
#define MINILIB_H

void str_cat(char s1[], char s2[]);
int str_len(char s[]);
void str_cpy(char s1[], char s2[]);
void int2str(int n, char s[]);
int str2int(char s[]);
int str_cmp(char s1[], char s2[]);
int str_count(char s[], char c);
void str_append(char s[], char c);

void kprintf(char *fmt, ...);

#endif
