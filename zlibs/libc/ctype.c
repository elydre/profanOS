/*****************************************************************************\
|   === ctype.c : 2024 ===                                                    |
|                                                                             |
|    Implementation of ctype functions from libC                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <ctype.h>

const char _ctype_[256] = {
    0,
    _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C,
    _C,     _S,     _S,     _S,     _S,     _S,     _C,     _C,
    _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C,
    _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C,
    _S,     _P,     _P,     _P,     _P,     _P,     _P,     _P,
    _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P,

    _N,     _N,     _N,     _N,     _N,     _N,     _N,     _N,
    _N,     _N,     _P,     _P,     _P,     _P,     _P,     _P,

    _P,     _U|_X,  _U|_X,  _U|_X,  _U|_X,  _U|_X,  _U|_X,  _U,
    _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U,
    _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U,
    _U,     _U,     _U,     _P,     _P,     _P,     _P,     _P,
    _P,     _L|_X,  _L|_X,  _L|_X,  _L|_X,  _L|_X,  _L|_X,  _L,
    _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L,
    _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L,
    _L,     _L,     _L,     _P,     _P,     _P,     _P,     _C
};

int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

int isalpha(int c) {
    return islower(c) || isupper(c);
}

int isascii(int c) {
    return c <= 127 && c >= 0;
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
