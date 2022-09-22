#include <string.h>

#include <stdint.h>

// K&R implementation

void int_to_ascii(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) str[i++] = '-';
    str[i] = '\0';

    reverse(str);
}

void hex_to_ascii(int n, char str[]) {
    str[0] = '\0';
    append(str, '0');
    append(str, 'x');
    char zeros = 0;

    int32_t tmp;
    int i;
    for (i = 28; i > 0; i -= 4) {
        tmp = (n >> i) & 0xF;
        if (tmp == 0 && zeros == 0) continue;
        zeros = 1;
        if (tmp > 0xA) append(str, tmp - 0xA + 'a');
        else append(str, tmp + '0');
    }

    tmp = n & 0xF;
    if (tmp >= 0xA) append(str, tmp - 0xA + 'a');
    else append(str, tmp + '0');
}

int in_string(char str[], char thing) {
    for (int i = 0; i < strlen(str);i++) {
        if (str[i] == thing) return 1;
    }
    return 0;
}

int count_string(char str[], char thing) {
    int total = 0;
    for (int i = 0; i < strlen(str);i++) {
        if (str[i] == thing) total++;
    }
    return total;
}

int ascii_to_int(char str[]) {
    int i, n;
    n = 0;
    for (i = 0; str[i] != '\0'; ++i)
        n = n * 10 + str[i] - '0';
    return n;
}

void reverse(char s[]) {
    int c, i, j;
    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int strlen(char s[]) {
    int i = 0;
    while (s[i] != '\0') ++i;
    return i;
}

void append(char s[], char n) {
    int len = strlen(s);
    s[len] = n;
    s[len+1] = '\0';
}

void backspace(char s[]) {
    int len = strlen(s);
    s[len-1] = '\0';
}

void strcpy(char s1[], char s2[]) {
    int i;
    for (i = 0; s2[i] != '\0'; ++i) {
        s1[i] = s2[i];
    }
    s1[i] = '\0';
}

// Returns <0 if s1<s2, 0 if s1==s2, >0 if s1>s2
int strcmp(char s1[], char s2[]) {
    if (strlen(s1) != strlen(s2)) {
        return -1;
    }
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

void str_start_split(char s[], char delim) {
    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] == delim) {
            s[i] = '\0';
            return;
        }
    }
}

void str_end_split(char s[], char delim) {
    int limit = 0;
    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] == delim) {
            limit = i + 1; break;
        }
    }

    for (int i = limit; s[i] != '\0'; i++) {
        s[i - limit] = s[i];
    }
    s[strlen(s) - limit] = '\0';    
}

char* strcat(char s1[], const char s2[]) {
    char *start = s1;
    while(*start != '\0') start++;
    while(*s2 != '\0') *start++ = *s2++;
    *start = '\0';
    return s1;
}
