/*****************************************************************************\
|   === vsnprintf.c : 2025 ===                                                |
|                                                                             |
|    Implementation of vsnprintf function from libC                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original code from sun NLnet Labs (see below)                 `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>

/* snprintf - compatibility implementation of snprintf, vsnprintf
 *
 * Copyright (c) 2013, NLnet Labs. All rights reserved.
 *
 * This software is open source.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the NLNET LABS nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Very portable snprintf implementation, limited in functionality,
 * esp. for %[capital] %[nonportable] and so on.  Reduced float functionality,
 * mostly in formatting and range (e+-16), for %f and %g.
 *
 * %s, %d, %u, %i, %x, %c, %n and %% are fully supported.
 *   This includes width, precision, flags 0- +, and *(arg for wid,prec).
 * %f, %g, %m, %p have reduced support, support for wid,prec,flags,*, but
 *   less floating point range, no %e formatting for %g.
*/

/** add padding to string */
static void print_pad(char **at, size_t * left, int *ret, char p, int num) {
    while (num--) {
        if (*left > 1) {
            *(*at)++ = p;
            (*left)--;
        }
        (*ret)++;
    }
}

/** get negative symbol, 0 if none */
static char get_negsign(int negative, int plus, int space) {
    if (negative)
        return '-';
    if (plus)
        return '+';
    if (space)
        return ' ';
    return 0;
}

#define PRINT_DEC_BUFSZ 32 /* 20 is enough for 64 bit decimals */
/** print decimal into buffer, returns length */
static int print_dec(char *buf, int max, unsigned int value) {
    int i = 0;
    if (value == 0) {
        if (max > 0) {
            buf[0] = '0';
            i = 1;
        }
    } else while (value && i < max) {
        buf[i++] = '0' + value % 10;
        value /= 10;
    }
    return i;
}

/** print long decimal into buffer, returns length */
static int print_dec_l(char *buf, int max, unsigned long value) {
    int i = 0;
    if (value == 0) {
        if (max > 0) {
            buf[0] = '0';
            i = 1;
        }
    } else while (value && i < max) {
        buf[i++] = '0' + value % 10;
        value /= 10;
    }
    return i;
}

/** print long decimal into buffer, returns length */
static int print_dec_ll(char *buf, int max, unsigned long long value) {
    int i = 0;
    if (value == 0) {
        if (max > 0) {
            buf[0] = '0';
            i = 1;
        }
    } else while (value && i < max) {
        buf[i++] = '0' + value % 10;
        value /= 10;
    }
    return i;
}

/** print hex into buffer, returns length */
static int print_hex(char *buf, int max, unsigned int value) {
    const char *h = "0123456789abcdef";
    int i = 0;
    if (value == 0) {
        if (max > 0) {
            buf[0] = '0';
            i = 1;
        }
    } else while (value && i < max) {
        buf[i++] = h[value & 0x0f];
        value >>= 4;
    }
    return i;
}

/** print long hex into buffer, returns length */
static int print_hex_l(char *buf, int max, unsigned long value) {
    const char *h = "0123456789abcdef";
    int i = 0;
    if (value == 0) {
        if (max > 0) {
            buf[0] = '0';
            i = 1;
        }
    } else while (value && i < max) {
        buf[i++] = h[value & 0x0f];
        value >>= 4;
    }
    return i;
}

/** print long long hex into buffer, returns length */
static int print_hex_ll(char *buf, int max, unsigned long long value) {
    const char *h = "0123456789abcdef";
    int i = 0;
    if (value == 0) {
        if (max > 0) {
            buf[0] = '0';
            i = 1;
        }
    } else while (value && i < max) {
        buf[i++] = h[value & 0x0f];
        value >>= 4;
    }
    return i;
}

/** copy string into result, reversed */
static void spool_str_rev(char **at, size_t *left, int *ret, const char *buf, int len) {
    int i = len;
    while (i) {
        if (*left > 1) {
            *(*at)++ = buf[--i];
            (*left)--;
        } else --i;
        (*ret)++;
    }
}

/** copy string into result */
static void spool_str(char **at, size_t *left, int *ret, const char *buf, int len) {
    int i;
    for (i = 0; i < len; i++) {
        if (*left > 1) {
            *(*at)++ = buf[i];
            (*left)--;
        }
        (*ret)++;
    }
}

/** copy wide string into result */
static void spool_lstr(char **at, size_t *left, int *ret, const wchar_t *buf, int len) {
    int i;
    for (i = 0; i < len; i++) {
        if (*left > 1) {
            *(*at)++ = (char) buf[i];
            (*left)--;
        }
        (*ret)++;
    }
}

/** print number formatted */
static void print_num(char **at, size_t *left, int *ret, int minw, int precision,
        int prgiven, int zeropad, int minus, int plus, int space,
        int zero, int negative, char *buf, int len)
{
    int w = len; /* excludes minus sign */
    char s = get_negsign(negative, plus, space);
    if (minus) {
        /* left adjust the number into the field, space padding */
        /* calc numw = [sign][zeroes][number] */
        int numw = w;
        if (precision == 0 && zero)
            numw = 0;
        if (numw < precision)
            numw = precision;
        if (s)
            numw++;

        /* sign */
        if (s)
            print_pad(at, left, ret, s, 1);

        /* number */
        if (precision == 0 && zero) {
            /* "" for the number */
        } else {
            if (w < precision)
                print_pad(at, left, ret, '0', precision - w);
            spool_str_rev(at, left, ret, buf, len);
        }
        /* spaces */
        if (numw < minw)
            print_pad(at, left, ret, ' ', minw - numw);
    } else {
        /* pad on the left of the number */
        /* calculate numw has width of [sign][zeroes][number] */
        int numw = w;
        if (precision == 0 && zero)
            numw = 0;
        if (numw < precision)
            numw = precision;
        if (!prgiven && zeropad && numw < minw)
            numw = minw;
        else if (s)
            numw++;

        /* pad with spaces */
        if (numw < minw)
            print_pad(at, left, ret, ' ', minw - numw);
        /* print sign (and one less zeropad if so) */
        if (s) {
            print_pad(at, left, ret, s, 1);
            numw--;
        }
        /* pad with zeroes */
        if (w < numw)
            print_pad(at, left, ret, '0', numw - w);
        if (precision == 0 && zero)
            return;
        /* print the characters for the value */
        spool_str_rev(at, left, ret, buf, len);
    }
}

/** print %d and %i */
static void print_num_d(char **at, size_t *left, int *ret, int value,
        int minw, int precision, int prgiven, int zeropad, int minus,
        int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = (value < 0);
    int zero = (value == 0);
    int len = print_dec(buf, (int)sizeof(buf),
            (unsigned int)(negative ? -value : value));
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
            plus, space, zero, negative, buf, len);
}

/** print %ld and %li */
static void print_num_ld(char **at, size_t *left, int *ret, long value,
        int minw, int precision, int prgiven, int zeropad, int minus,
        int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = (value < 0);
    int zero = (value == 0);
    int len = print_dec_l(buf, (int)sizeof(buf),
            (unsigned long)(negative ? -value : value));
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
            plus, space, zero, negative, buf, len);
}

/** print %lld and %lli */
static void print_num_lld(char **at, size_t * left, int *ret, long long value,
        int minw, int precision, int prgiven, int zeropad, int minus,
        int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = (value < 0);
    int zero = (value == 0);
    int len = print_dec_ll(buf, (int)sizeof(buf),
            (unsigned long long)(negative ? -value : value));
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
            plus, space, zero, negative, buf, len);
}

/** print %u */
static void print_num_u(char **at, size_t * left, int *ret, unsigned int value,
        int minw, int precision, int prgiven, int zeropad, int minus,
        int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
    int len = print_dec(buf, (int)sizeof(buf), value);
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
            plus, space, zero, negative, buf, len);
}

/** print %lu */
static void print_num_lu(char **at, size_t * left, int *ret, unsigned long value,
        int minw, int precision, int prgiven, int zeropad, int minus,
        int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
    int len = print_dec_l(buf, (int)sizeof(buf), value);
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
            plus, space, zero, negative, buf, len);
}

/** print %llu */
static void print_num_llu(char **at, size_t * left, int *ret, unsigned long long value,
        int minw, int precision, int prgiven, int zeropad, int minus,
        int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
    int len = print_dec_ll(buf, (int)sizeof(buf), value);
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
            plus, space, zero, negative, buf, len);
}

/** print %x */
static void print_num_x(char **at, size_t * left, int *ret, unsigned int value,
        int minw, int precision, int prgiven, int zeropad, int minus,
        int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
    int len = print_hex(buf, (int)sizeof(buf), value);
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
            plus, space, zero, negative, buf, len);
}

/** print %lx */
static void print_num_lx(char **at, size_t * left, int *ret, unsigned long value,
        int minw, int precision, int prgiven, int zeropad, int minus,
        int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
    int len = print_hex_l(buf, (int)sizeof(buf), value);
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
            plus, space, zero, negative, buf, len);
}

/** print %llx */
static void print_num_llx(char **at, size_t * left, int *ret, unsigned long long value,
        int minw, int precision, int prgiven, int zeropad, int minus,
        int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
    int len = print_hex_ll(buf, (int)sizeof(buf), value);
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
            plus, space, zero, negative, buf, len);
}

/** print %llp */
static void print_num_llp(char **at, size_t * left, int *ret, void *value,
        int minw, int precision, int prgiven, int zeropad, int minus,
        int plus, int space)
{
    char buf[PRINT_DEC_BUFSZ];
    int negative = 0;
    int zero = (value == 0);
#if defined(SIZE_MAX) && defined(UINT32_MAX) && (UINT32_MAX == SIZE_MAX || INT32_MAX == SIZE_MAX)
    /* avoid warning about upcast on 32bit systems */
    unsigned long long llvalue = (unsigned long) value;
#else
    unsigned long long llvalue = (unsigned long long)value;
#endif
    int len = print_hex_ll(buf, (int)sizeof(buf), llvalue);
    if (zero) {
        buf[0] = ')';
        buf[1] = 'l';
        buf[2] = 'i';
        buf[3] = 'n';
        buf[4] = '(';
    len = 5;
    } else {
        /* put '0x' in front of the (reversed) buffer result */
        if (len < PRINT_DEC_BUFSZ)
            buf[len++] = 'x';
        if (len < PRINT_DEC_BUFSZ)
            buf[len++] = '0';
    }
    print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
            plus, space, zero, negative, buf, len);
}

#define PRINT_FLOAT_BUFSZ 64 /* xx.yy with 20.20 about the max */
/** spool remainder after the decimal point to buffer, in reverse */

static int print_remainder(char *buf, int max, double r, int prec) {
    unsigned long long cap = 1;
    unsigned long long value;
    int len, i;

    if (prec > 19)
        prec = 19; // max we can do

    if (max < prec)
        return 0;

    for (i = 0; i < prec; i++)
        cap *= 10;

    r *= (double)cap;
    value = (unsigned long long)r;

    /* see if we need to round up */
    if (((unsigned long long)((r - (double)value) * 10.0)) >= 5) {
        value++;
        /* that might carry to numbers before the comma, if so,
        * just ignore that rounding. failure because 64bitprintout */
        if (value >= cap)
            value = cap - 1;
    }

    len = print_dec_ll(buf, max, value);

    /* pad with zeroes, e.g. if 0.0012 */
    while (len < prec)
        buf[len++] = '0';

    if (len < max)
        buf[len++] = '.';

    return len;
}

/** spool floating point to buffer */
static int print_float(char *buf, int max, double value, int prec) {
    /* as xxx.xxx  if prec==0, no '.', with prec decimals after . */
    /* no conversion for NAN and INF, because we do not want to require
       linking with -lm. */
    /* Thus, the conversions use 64bit integers to convert the numbers,
     * which makes 19 digits before and after the decimal point the max */
    unsigned long long whole = (unsigned long long)value;
    double remain = value - (double)whole;
    int len = 0;
    if (prec != 0)
    len = print_remainder(buf, max, remain, prec);
    len += print_dec_ll(buf + len, max - len, whole);
    return len;
}

/** print %f */
static void print_num_f(char **at, size_t *left, int *ret, double value,
        int minw, int precision, int prgiven, int zeropad, int minus,
        int plus, int space)
{
    char buf[PRINT_FLOAT_BUFSZ];
    int negative = (value < 0);
    int zero = 0;
    int len;
    if (!prgiven)
        precision = 6;
    len = print_float(buf, (int)sizeof(buf), negative ? -value : value,
              precision);
    print_num(at, left, ret, minw, 1, 0, zeropad, minus,
            plus, space, zero, negative, buf, len);
}

/* rudimentary %g support */
static int print_float_g(char *buf, int max, double value, int prec) {
    unsigned long long whole = (unsigned long long)value;
    double remain = value - (double)whole;
    int before = 0;
    int len = 0;

    /* number of digits before the decimal point */
    while (whole > 0) {
        before++;
        whole /= 10;
    }
    whole = (unsigned long long)value;

    if (prec > before && remain != 0.0) {
        /* see if the last decimals are zero, if so, skip them */
        len = print_remainder(buf, max, remain, prec - before);
        while (len > 0 && buf[0] == '0') {
            memmove(buf, buf + 1, --len);
        }
    }
    len += print_dec_ll(buf + len, max - len, whole);
    return len;
}

/** print %g */
static void print_num_g(char **at, size_t *left, int *ret, double value,
        int minw, int precision, int prgiven, int zeropad, int minus,
        int plus, int space)
{
    char buf[PRINT_FLOAT_BUFSZ];
    int negative = (value < 0);
    int zero = 0;
    int len;
    if (!prgiven)
        precision = 6;
    if (precision == 0)
        precision = 1;
    len = print_float_g(buf, (int)sizeof(buf), negative ? -value : value,
            precision);
    print_num(at, left, ret, minw, 1, 0, zeropad, minus,
            plus, space, zero, negative, buf, len);
}

/** print %s */
static void print_str(char **at, size_t * left, int *ret, char *s,
        int minw, int precision, int prgiven, int minus)
{
    int w;

    if (s == NULL)
        s = "(null)";

    /* with prec: no more than x characters from this string, stop at 0 */
    if (prgiven)
        w = strnlen(s, precision);
    else
        w = (int) strlen(s); // up to the null

    if (w < minw && !minus)
        print_pad(at, left, ret, ' ', minw - w);

    spool_str(at, left, ret, s, w);

    if (w < minw && minus)
        print_pad(at, left, ret, ' ', minw - w);
}

/** print %ls */
static void print_lstr(char **at, size_t * left, int *ret, wchar_t *s,
        int minw, int precision, int prgiven, int minus)
{
    int w;

    if (s == NULL)
        s = L"(null)";

    /* with prec: no more than x characters from this string, stop at 0 */
    if (prgiven)
        w = wcsnlen(s, precision);
    else
        w = (int) wcslen(s); // up to the null

    if (w < minw && !minus)
        print_pad(at, left, ret, ' ', minw - w);

    spool_lstr(at, left, ret, s, w);

    if (w < minw && minus)
        print_pad(at, left, ret, ' ', minw - w);
}


/** print %c */
static void print_char(char **at, size_t *left, int *ret, int c, int minw, int minus) {
    if (1 < minw && !minus)
        print_pad(at, left, ret, ' ', minw - 1);
    print_pad(at, left, ret, c, 1);
    if (1 < minw && minus)
        print_pad(at, left, ret, ' ', minw - 1);
}

/**
 * Print to string.
 * str: string buffer for result. result will be null terminated.
 * size: size of the buffer. null is put inside buffer.
 * format: printf format string.
 * arg: '...' arguments to print.
 * returns number of characters. a null is printed after this.
 * return number of bytes that would have been written
 *     if the buffer had been large enough.
 *
 * supported format specifiers:
 *  %s, %u, %d, %x, %i, %f, %g, %c, %p, %n.
 *  length: l, ll (for d, u, x).
 *  precision: 6.6d (for d, u, x)
 *      %f, %g precisions, 0.3f
 *      %20s, '.*s'
 *  and %%.
 */

int vsnprintf(char *str, size_t size, const char *format, va_list arg) {
    char *at = str;
    size_t left = size;
    int ret = 0;
    const char *fmt = format;
    int conv, minw, precision, prgiven, zeropad, minus, plus, space, length;

    while (*fmt) {
        /* copy string before % */
        while (*fmt && *fmt != '%') {
            if (left > 1) {
                *at++ = *fmt++;
                left--;
            } else
                fmt++;
            ret++;
        }

        /* see if we are at end */
        if (!*fmt)
            break;

        /* fetch next argument % designation from format string */
        fmt++; /* skip the '%' */

            /********************************/
        /* get the argument designation */
            /********************************/
        /* we must do this vararg stuff inside this function for
        * portability.  Hence, get_designation, and print_designation
        * are not their own functions. */

        /* printout designation:
        * conversion specifier: x, d, u, s, c, n, m, p
        * flags: # not supported
        *        0 zeropad (on the left)
        *        - left adjust (right by default)
        *        ' ' printspace for positive number (in - position).
        *        + alwayssign
        * fieldwidth: [1-9][0-9]* minimum field width.
        *      if this is * then type int next argument specifies the minwidth.
        *      if this is negative, the - flag is set (with positive width).
        * precision: period[digits]*, %.2x.
        *      if this is * then type int next argument specifies the precision.
        *      just '.' or negative value means precision=0.
        *              this is mindigits to print for d, i, u, x
        *              this is aftercomma digits for f
        *              this is max number significant digits for g
        *              maxnumber characters to be printed for s
        * length: 0-none (int), 1-l (long), 2-ll (long long)
        *      notsupported: hh (char), h (short), L (long double), q, j, z, t
        * Does not support %m$ and *m$ argument designation as array indices.
        * Does not support %#x
        *
        */

        minw = 0;
        precision = 1;
        prgiven = 0;
        zeropad = 0;
        minus = 0;
        plus = 0;
        space = 0;
        length = 0;

        /* get flags in any order */
        for (;;) {
            if (*fmt == '0')
                zeropad = 1;
            else if (*fmt == '-')
                minus = 1;
            else if (*fmt == '+')
                plus = 1;
            else if (*fmt == ' ')
                space = 1;
            else
                break;
            fmt++;
        }

        /* field width */
        if (*fmt == '*') {
            fmt++; // skip char
            minw = va_arg(arg, int);
            if (minw < 0) {
                minus = 1;
                minw = -minw;
            }
        } else while (*fmt >= '0' && *fmt <= '9') {
            minw = minw * 10 + (*fmt++) - '0';
        }

        /* precision */
        if (*fmt == '.') {
            fmt++; // skip period
            prgiven = 1;
            precision = 0;
            if (*fmt == '*') {
                fmt++; // skip char
                precision = va_arg(arg, int);
                if (precision < 0)
                    precision = 0;
            } else while (*fmt >= '0' && *fmt <= '9') {
                precision = precision * 10 + (*fmt++) - '0';
            }
        }

        /* length */
        if (*fmt == 'l') {
            fmt++; // skip char
            length = 1;
            if (*fmt == 'l') {
                fmt++; // skip char
                length = 2;
            }
        }

        /* get the conversion */
        if (!*fmt)
            conv = 0;
        else
            conv = *fmt++;

            /***********************************/
        /* print that argument designation */
            /***********************************/
        switch (conv) {
        case 'i':
        case 'd':
            if (length == 0)
                print_num_d(&at, &left, &ret,
                        va_arg(arg, int),
                        minw, precision, prgiven, zeropad, minus, plus, space);
            else if (length == 1)
                print_num_ld(&at, &left, &ret,
                        va_arg(arg, long),
                        minw, precision, prgiven, zeropad, minus, plus, space);
            else if (length == 2)
                print_num_lld(&at, &left, &ret,
                        va_arg(arg, long long),
                        minw, precision, prgiven, zeropad, minus, plus, space);
            break;
        case 'u':
            if (length == 0)
                print_num_u(&at, &left, &ret,
                        va_arg(arg, unsigned int),
                        minw, precision, prgiven, zeropad, minus, plus, space);
            else if (length == 1)
                print_num_lu(&at, &left, &ret,
                        va_arg(arg, unsigned long),
                        minw, precision, prgiven, zeropad, minus, plus, space);
            else if (length == 2)
                print_num_llu(&at, &left, &ret,
                        va_arg(arg, unsigned long long),
                        minw, precision, prgiven, zeropad, minus, plus, space);
            break;
        case 'x':
            if (length == 0)
                print_num_x(&at, &left, &ret,
                        va_arg(arg, unsigned int),
                        minw, precision, prgiven, zeropad, minus, plus, space);
            else if (length == 1)
                print_num_lx(&at, &left, &ret,
                        va_arg(arg, unsigned long),
                        minw, precision, prgiven, zeropad, minus, plus, space);
            else if (length == 2)
                print_num_llx(&at, &left, &ret,
                        va_arg(arg, unsigned long long),
                        minw, precision, prgiven, zeropad, minus, plus, space);
            break;
        case 's':
            if (length != 1)
                print_str(&at, &left, &ret, va_arg(arg, char *),
                        minw, precision, prgiven, minus);
            else
                print_lstr(&at, &left, &ret, va_arg(arg, wchar_t *),
                        minw, precision, prgiven, minus);
            break;
        case 'c':
            print_char(&at, &left, &ret, va_arg(arg, int), minw, minus);
            break;
        case 'n':
            *va_arg(arg, int *) = ret;
            break;
        case 'm':
            print_str(&at, &left, &ret, strerror(errno),
                minw, precision, prgiven, minus);
            break;
        case 'p':
            print_num_llp(&at, &left, &ret, va_arg(arg, void *),
                minw, precision, prgiven, zeropad, minus, plus, space);
            break;
        case '%':
            print_pad(&at, &left, &ret, '%', 1);
            break;
        case 'f':
            print_num_f(&at, &left, &ret, va_arg(arg, double),
                minw, precision, prgiven, zeropad, minus, plus, space);
            break;
        case 'g':
            print_num_g(&at, &left, &ret, va_arg(arg, double),
                minw, precision, prgiven, zeropad, minus, plus, space);
            break;
        /* unknown */
        default:
            break;
        }
    }

    /* zero terminate */
    if (left > 0)
        *at = 0;
    return ret;
}
