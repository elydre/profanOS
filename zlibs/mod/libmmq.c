#include <syscall.h>
#include <type.h>

#define malloc(size) ((void *) c_mem_alloc((size), 0, 1))

int main(void) {
    return 0;
}

void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);


void *calloc(uint32_t nmemb, uint32_t lsize) {
    uint32_t size = lsize * nmemb;
    int addr = c_mem_alloc(size, 0, 1);
    if (addr == 0)
        return NULL;
    memset((uint8_t *) addr, 0, size);
    return (void *) addr;
}

void free(void *mem) {
    if (mem == NULL)
        return;
    c_mem_free_addr((int) mem);
}

void *realloc(void *mem, uint32_t new_size) {
    if (mem == NULL)
        return malloc(new_size);

    uint32_t old_size = c_mem_get_alloc_size((uint32_t) mem);
    uint32_t new_addr = c_mem_alloc(new_size, 0, 1);
    if (new_addr == 0)
        return NULL;

    memcpy((uint8_t *) new_addr, (uint8_t *) mem, old_size < new_size ? old_size : new_size);
    free(mem);
    return (void *) new_addr;
}

void *memcpy(void *dest, const void *src, size_t n) {
    register uint8_t *d = (uint8_t *) dest;
    register const uint8_t *s = (const uint8_t *) src;

    while (n--) {
        *d++ = *s++;
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    register const uint8_t *r1 = (const uint8_t *) s1;
    register const uint8_t *r2 = (const uint8_t *) s2;

    int r = 0;

    while (n-- && ((r = ((int)(*r1++)) - *r2++) == 0));
    return r;
}

void *memset(void *s, int c, size_t n) {
    register uint8_t *p = (uint8_t *) s;
    register uint8_t v = (uint8_t) c;

    while (n--) {
        *p++ = v;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    register uint8_t *d = (uint8_t *) dest;
    register const uint8_t *s = (const uint8_t *) src;

    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }

    return dest;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *(unsigned char *) s1 - *(unsigned char *) s2;
}

int strcpy(char *dest, const char *src) {
    while ((*dest++ = *src++));
    return 0;
}

size_t strlen(const char *s) {
    size_t len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

char *strdup(const char *s) {
    size_t len = strlen(s);
    char *d = malloc(len + 1);
    if (d == NULL)
        return NULL;
    memcpy(d, s, len);
    d[len] = '\0';
    return d;
}
