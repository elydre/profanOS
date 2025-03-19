/*****************************************************************************\
|   === fnmatch.c : 2025 ===                                                  |
|                                                                             |
|    Implementation of fnmatch function from libC                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original code from the GNU C Library                          `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <fnmatch.h>

#undef  FOLD
#define FOLD(c) (((flags & FNM_CASEFOLD) && (c) >= 'A' && (c) <= 'Z') ? (c) - 'A' + 'a' : (c))

int fnmatch(const char *pattern, const char *string, int flags) {
    register const char *p = pattern, *n = string;
    register unsigned char c;

    while ((c = *p++) != '\0') {
        c = FOLD(c);

        switch (c) {
            case '?': {
                if (*n == '\0')
                    return FNM_NOMATCH;
                else if ((flags & FNM_FILE_NAME) && *n == '/')
                    return FNM_NOMATCH;
                else if ((flags & FNM_PERIOD) && *n == '.' &&
                        (n == string || ((flags & FNM_FILE_NAME) && n[-1] == '/')))
                    return FNM_NOMATCH;
                break;
            }

            case '\\': {
                if (!(flags & FNM_NOESCAPE)) {
                    c = *p++;
                    c = FOLD(c);
                }

                if (FOLD((unsigned char) *n) != c)
                    return FNM_NOMATCH;
                break;
            }

            case '*': {
                if ((flags & FNM_PERIOD) && *n == '.' &&
                    (n == string || ((flags & FNM_FILE_NAME) && n[-1] == '/')))
                    return FNM_NOMATCH;

                for (c = *p++; c == '?' || c == '*'; c = *p++, ++n)
                    if (((flags & FNM_FILE_NAME) && *n == '/') ||
                            (c == '?' && *n == '\0'))
                        return FNM_NOMATCH;

                if (c == '\0')
                    return 0;

                unsigned char c1 = (!(flags & FNM_NOESCAPE) && c == '\\') ? *p : c;
                c1 = FOLD(c1);
                for (--p; *n != '\0'; ++n)
                    if ((c == '[' || FOLD((unsigned char) *n) == c1) &&
                            fnmatch (p, n, flags & ~FNM_PERIOD) == 0)
                        return 0;
                return FNM_NOMATCH;
            }

            case '[': {
                // Nonzero if the sense of the character class is inverted.
                register int negate;

                if (*n == '\0')
                    return FNM_NOMATCH;

                if ((flags & FNM_PERIOD) && *n == '.' &&
                        (n == string || ((flags & FNM_FILE_NAME) && n[-1] == '/')))
                    return FNM_NOMATCH;

                negate = (*p == '!' || *p == '^');
                if (negate)
                    ++p;

                c = *p++;
                for (;;) {
                    register unsigned char cstart = c, cend = c;

                    if (!(flags & FNM_NOESCAPE) && c == '\\')
                        cstart = cend = *p++;

                    cstart = cend = FOLD(cstart);

                    // [ (unterminated) loses
                    if (c == '\0')
                        return FNM_NOMATCH;

                    c = *p++;
                    c = FOLD(c);

                    // [/] can never match
                    if ((flags & FNM_FILE_NAME) && c == '/')
                        return FNM_NOMATCH;

                    if (c == '-' && *p != ']') {
                        cend = *p++;
                        if (!(flags & FNM_NOESCAPE) && cend == '\\')
                            cend = *p++;
                        if (cend == '\0')
                            return FNM_NOMATCH;
                        cend = FOLD(cend);
                        c = *p++;
                    }

                    if (FOLD((unsigned char) *n) >= cstart
                            && FOLD((unsigned char) *n) <= cend)
                        goto matched;

                    if (c == ']')
                        break;
                }

                if (!negate)
                    return FNM_NOMATCH;
                break;

                matched:

                // Skip the rest of the [...] that already matched.
                while (c != ']') {
                    // [... (unterminated) loses.
                    if (c == '\0')
                        return FNM_NOMATCH;

                    c = *p++;

                    // XXX 1003.2d11 is unclear if this is right.
                    if (!(flags & FNM_NOESCAPE) && c == '\\')
                        ++p;
                }

                if (negate)
                    return FNM_NOMATCH;

                break;
            }

            default:
                if (c != FOLD((unsigned char) *n))
                    return FNM_NOMATCH;
        }
        n++;
    }

    if (*n == '\0')
        return 0;

    // The FNM_LEADING_DIR flag says that "foo*" matches "foobar/frobozz".
    if ((flags & FNM_LEADING_DIR) && *n == '/')
        return 0;

    return FNM_NOMATCH;
}
