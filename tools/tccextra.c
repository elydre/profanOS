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
