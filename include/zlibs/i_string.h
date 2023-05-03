#ifndef I_STRING_ID
#define I_STRING_ID 1001

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

/*
void int_to_ascii(int n, char str[]);
void hex_to_ascii(int n, char str[]);
void double_to_ascii(double n, char str[]);
int  ascii_to_int(char str[]);
int  str_count(char str[], char thing);
void str_reverse(char s[]);
int  str_len(char s[]);
void str_append(char s[], char n);
void str_cpy(char s1[], char s2[]);
int  str_cmp(char s1[], char s2[]);
void str_start_split(char s[], char delim);
void str_end_split(char s[], char delim);
void str_cat(char s1[], char s2[]);
void str_delchar(char s[], char c);
int  str_in_str(char s1[], char s2[]);
*/

#define int_to_ascii ((void (*)(int, char *)) get_func_addr(I_STRING_ID, 2))
#define hex_to_ascii ((void (*)(int, char *)) get_func_addr(I_STRING_ID, 3))
#define double_to_ascii ((void (*)(double, char *)) get_func_addr(I_STRING_ID, 4))
#define ascii_to_int ((int (*)(char *)) get_func_addr(I_STRING_ID, 5))
#define str_count ((int (*)(char *, char)) get_func_addr(I_STRING_ID, 6))
#define str_reverse ((void (*)(char *)) get_func_addr(I_STRING_ID, 7))
#define str_len ((int (*)(char *)) get_func_addr(I_STRING_ID, 8))
#define str_append ((void (*)(char *, char)) get_func_addr(I_STRING_ID, 9))
#define str_cpy ((void (*)(char *, char *)) get_func_addr(I_STRING_ID, 10))
#define str_cmp ((int (*)(char *, char *)) get_func_addr(I_STRING_ID, 11))
#define str_start_split ((void (*)(char *, char)) get_func_addr(I_STRING_ID, 12))
#define str_end_split ((void (*)(char *, char)) get_func_addr(I_STRING_ID, 13))
#define str_cat ((void (*)(char *, char *)) get_func_addr(I_STRING_ID, 14))
#define str_delchar ((void (*)(char *, char)) get_func_addr(I_STRING_ID, 15))
#define str_in_str ((int (*)(char *, char *)) get_func_addr(I_STRING_ID, 16))

#endif
