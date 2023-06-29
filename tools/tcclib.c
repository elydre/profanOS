// memset
void *memset(void *s, int c, unsigned int n) {
    char *p = (char *)s;
    while (n--)
        *p++ = c;
    return s;
}

// memcpy
void *memcpy(void *dest, const void *src, unsigned int n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    while (n--)
        *d++ = *s++;
    return dest;
}
