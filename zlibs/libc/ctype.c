#include <ctype.h>

int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

int isalpha(int c) {
    return islower(c) || isupper(c);
}

int iscntrl(int c) {
    return (c >= 0 && c <= 0x1F) || c == 0x7F;
}

int isdigit(int c) {
    return c >= '0' && c <= '9';
}

int isgraph(int c) {
    return c >= 0x21 && c <= 0x7E;
}

int islower(int c) {
    return c >= 'a' && c <= 'z';
}

int isprint(int c) {
    return c >= 0x20 && c <= 0x7E;
}

int ispunct(int c) {
    return isgraph(c) && !isalnum(c);
}

int isspace(int c) {
    return c == ' ' || (c >= 0x09 && c <= 0x0D);
}

int isupper(int c) {
    return c >= 'A' && c <= 'Z';
}

int isxdigit(int c) {
    return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

int tolower(int c) {
    if (isupper(c)) {
        return c + 0x20;
    }
    return c;
}

int toupper(int c) {
    if (islower(c)) {
        return c - 0x20;
    }
    return c;
}
