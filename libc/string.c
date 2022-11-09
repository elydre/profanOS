#include <function.h>
#include <string.h>
#include <type.h>

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

    str_reverse(str);
}

void hex_to_ascii(int n, char str[]) {
    str[0] = '\0';
    str_append(str, '0');
    str_append(str, 'x');
    char zeros = 0;

    int tmp;
    int i;
    for (i = 28; i > 0; i -= 4) {
        tmp = (n >> i) & 0xF;
        if (tmp == 0 && zeros == 0) continue;
        zeros = 1;
        if (tmp > 0xA) str_append(str, tmp - 0xA + 'a');
        else str_append(str, tmp + '0');
    }

    tmp = n & 0xF;
    if (tmp >= 0xA) str_append(str, tmp - 0xA + 'a');
    else str_append(str, tmp + '0');
}

void double_to_ascii(double n, char str[]) {
    int int_part = (int) n;
    int frac_part = (int)((n - int_part) * pow(10, 6));
    int_to_ascii(int_part, str);
    int len = str_len(str);
    str[len] = '.';
    str[len + 1] = '\0';
    int_to_ascii(frac_part, str + len + 1);
    while (str[str_len(str) - 1] == '0') str[str_len(str) - 1] = '\0';
    str_append(str, '0');
}

int ascii_to_int(char str[]) {
    int i, n;
    n = 0;
    for (i = 0; str[i] != '\0'; ++i)
        n = n * 10 + str[i] - '0';
    return n;
}

int str_is_in(char str[], char thing) {
    for (int i = 0; i < str_len(str);i++) {
        if (str[i] == thing) return 1;
    }
    return 0;
}

int str_count(char str[], char thing) {
    int total = 0;
    for (int i = 0; i < str_len(str);i++) {
        if (str[i] == thing) total++;
    }
    return total;
}

void str_reverse(char s[]) {
    int c, i, j;
    for (i = 0, j = str_len(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int str_len(char s[]) {
    int i = 0;
    while (s[i] != '\0') ++i;
    return i;
}

void str_append(char s[], char n) {
    int len = str_len(s);
    s[len] = n;
    s[len+1] = '\0';
}

void str_backspace(char s[]) {
    int len = str_len(s);
    s[len-1] = '\0';
}

void str_cpy(char s1[], char s2[]) {
    int i;
    for (i = 0; s2[i] != '\0'; ++i) {
        s1[i] = s2[i];
    }
    s1[i] = '\0';
}

// Returns <0 if s1<s2, 0 if s1==s2, >0 if s1>s2
int str_cmp(char s1[], char s2[]) {
    if (str_len(s1) != str_len(s2)) {
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
    s[str_len(s) - limit] = '\0';    
}

void str_cat(char s1[], char s2[]) {
    char *start = s1;
    while(*start != '\0') start++;
    while(*s2 != '\0') *start++ = *s2++;
    *start = '\0';
}

void str_delchar(char s[], char c) {
    int i = 0;
    while (s[i] != '\0') {
        if (s[i] == c) {
            for (int j = i; s[j] != '\0'; j++) {
                s[j] = s[j+1];
            }
        } else {
            i++;
        }
    }
}

int str_in_str(char s1[], char s2[]) {
    // return 1 if s2 is in s1
    int s1_len = str_len(s1);
    int s2_len = str_len(s2);
    for (int i = 0; i < s1_len - s2_len; i++) {
        int j = 0;
        while (j < s2_len && s1[i + j] == s2[j]) j++;
        if (j == s2_len) return 1;
    }
    return 0;
}
