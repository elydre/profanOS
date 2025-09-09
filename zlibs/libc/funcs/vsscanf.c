/*****************************************************************************\
|   === vsscanf.c : 2025 ===                                                  |
|                                                                             |
|    Implementation of vsscanf function from libC                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original code from libslack UNIX library                      `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>

#define COPY                         *b++ = *s++, --width
#define MATCH(cond)                  if (width && (cond)) COPY;
#define MATCH_ACTION(cond, action)   if (width && (cond)) { COPY; action; }
#define MATCHES_ACTION(cond, action) while (width && (cond)) { COPY; action; }
#define FAIL                         (cnv) ? cnv : EOF

int vsscanf(const char *str, const char *format, va_list args) {
    const char *f, *s;
    int cnv = 0;

    for (s = str, f = format; *f; ++f) {
        if (*f == '%') {
            int size = 0;
            int width = 0;
            int do_cnv = 1;

            if (*++f == '*')
                ++f, do_cnv = 0;

            for (; isdigit((int)(unsigned int)*f); ++f)
                width *= 10, width += *f - '0';

            if (*f == 'h' || *f == 'l' || *f == 'L')
                size = *f++;

            if (*f != '[' && *f != 'c' && *f != 'n')
                while (isspace((int)(unsigned int)*s))
                    ++s;

            switch (*f) {
                case 'd':
                case 'i':
                case 'o':
                case 'u':
                case 'x':
                case 'X':
                case 'p': {
                    static const char types[] = "diouxXp";
                    static const int bases[] = { 10, 0, 8, 10, 16, 16, 16 };
                    static const int setsizes[] = { 10, 0, 0, 0, 0, 0, 0, 0, 8, 0, 10, 0, 0, 0, 0, 0, 22 };

                    int base = bases[strchr(types, *f) - types];
                    int setsize;
                    char buf[513];
                    char *b = buf;
                    int digit = 0;

                    if (width <= 0 || width > 512)
                        width = 512;

                    MATCH(*s == '+' || *s == '-')
                    MATCH_ACTION(*s == '0',
                        digit = 1;
                        MATCH_ACTION((*s == 'x' || *s == 'X') && (base == 0 || base == 16), base = 16) else base = 8;
                    )

                    setsize = setsizes[base];
                    MATCHES_ACTION(memchr("0123456789abcdefABCDEF", *s, setsize), digit = 1)

                    if (!digit)
                        return FAIL;

                    *b = '\0';
                    if (do_cnv) {
                        if (*f == 'd' || *f == 'i') {
                            long data = strtol(buf, NULL, base);
                            if (size == 'h')
                                *va_arg(args, short *) = (short)data;
                            else if (size == 'l')
                                *va_arg(args, long *) = data;
                            else
                                *va_arg(args, int *) = (int)data;
                        } else {
                            unsigned long data = strtoul(buf, NULL, base);
                            if (size == 'p')
                                *va_arg(args, void **) = (void *)data;
                            else if (size == 'h')
                                *va_arg(args, unsigned short *) = (unsigned short)data;
                            else if (size == 'l')
                                *va_arg(args, unsigned long *) = data;
                            else
                                *va_arg(args, unsigned int *) = (unsigned int)data;
                        }
                        ++cnv;
                    }

                    break;
                }

                case 'e':
                case 'E':
                case 'f':
                case 'g':
                case 'G': {
                    char buf[513];
                    char *b = buf;
                    int digit = 0;

                    if (width <= 0 || width > 512)
                        width = 512;

                    MATCH(*s == '+' || *s == '-')
                    MATCHES_ACTION(isdigit((int)(unsigned int)*s), digit = 1)
                    MATCH(*s == '.')
                    MATCHES_ACTION(isdigit((int)(unsigned int)*s), digit = 1)
                    MATCHES_ACTION(digit && (*s == 'e' || *s == 'E'),
                        MATCH(*s == '+' || *s == '-')
                        digit = 0;
                        MATCHES_ACTION(isdigit((int)(unsigned int)*s), digit = 1)
                    )

                    if (!digit)
                        return FAIL;

                    *b = '\0';
                    if (do_cnv) {
                        double data = strtod(buf, NULL);
                        if (size == 'l')
                            *va_arg(args, double *) = data;
                        else if (size == 'L')
                            *va_arg(args, long double *) = (long double)data;
                        else
                            *va_arg(args, float *) = (float)data;
                        ++cnv;
                    }

                    break;
                }

                case 's': {
                    char *arg = va_arg(args, char *);

                    if (width <= 0)
                        width = INT_MAX;

                    while (width-- && *s && !isspace((int)(unsigned int)*s)) {
                        if (do_cnv)
                            *arg++ = *s++;
                    }

                    if (do_cnv) {
                        *arg = '\0';
                        ++cnv;
                    }

                    break;
                }

                case '[': {
                    char *arg = va_arg(args, char *);
                    int setcomp = 0;
                    size_t setsize;
                    const char *end;

                    if (width <= 0)
                        width = INT_MAX;

                    if (*++f == '^') {
                        setcomp = 1;
                        ++f;
                    }

                    end = strchr((*f == ']') ? f + 1 : f, ']');

                    if (!end)
                        return FAIL;

                    setsize = end - f;

                    while (width-- && *s) {
                        if (!setcomp && !memchr(f, *s, setsize))
                            break;
                        if (setcomp && memchr(f, *s, setsize))
                            break;
                        if (do_cnv)
                            *arg++ = *s++;
                    }

                    if (do_cnv) {
                        *arg = '\0';
                        ++cnv;
                    }

                    f = end;
                    break;
                }

                case 'c': {
                    char *arg = va_arg(args, char *);

                    if (width <= 0)
                        width = 1;

                    while (width--) {
                        if (!*s)
                            return FAIL;
                        if (do_cnv)
                            *arg++ = *s++;
                    }

                    if (do_cnv)
                        ++cnv;

                    break;
                }

                case 'n': {
                    if (size == 'h')
                        *va_arg(args, short *) = (short)(s - str);
                    else if (size == 'l')
                        *va_arg(args, long *) = (long)(s - str);
                    else
                        *va_arg(args, int *) = (int)(s - str);
                    break;
                }

                case '%': {
                    if (*s++ != '%')
                        return cnv;
                    break;
                }

                default:
                    return FAIL;
            }
        }

        else if (isspace((int)(unsigned int)*f)) {
            while (isspace((int)(unsigned int)f[1]))
                ++f;
            while (isspace((int)(unsigned int)*s))
                ++s;
        }

        else if (*s++ != *f) {
            return cnv;
        }
    }

    return cnv;
}
