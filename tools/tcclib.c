// memset
void *memset(void *s, int c, unsigned int n) {
    char *p = (char *) s;
    while (n--)
        *p++ = c;
    return s;
}

// memcpy
void *memcpy(void *dest, const void *src, unsigned int n) {
    char *d = (char *) dest;
    const char *s = (const char *)src;
    while (n--)
        *d++ = *s++;
    return dest;
}

// memmove
void *memmove(void *dest, const void *src, unsigned int n) {
    char *d = (char *) dest;
    char *s = (char *) src;
    if (d < s)
        while (n--)
            *d++ = *s++;
    else {
        char *lasts = s + (n - 1);
        char *lastd = d + (n - 1);
        while (n--)
            *lastd-- = *lasts--;
    }
    return dest;
}

// __fixxfdi
long long __fixxfdi(long double x) {
    return (long long) x;
}

// __fixdfdi
long long __fixdfdi(double x) {
    return (long long) x;
}

// __ashldi3
long long __ashldi3(long long u, unsigned int b) {
    return u << b;
}

// __ashrdi3
long long __ashrdi3(long long u, unsigned int b) {
    return u >> b;
}
