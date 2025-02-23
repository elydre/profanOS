/*****************************************************************************\
|   === minilib.c : 2024 ===                                                  |
|                                                                             |
|    Kernel mini-library functions                                 .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/snowflake.h>
#include <drivers/keyboard.h>
#include <drivers/serial.h>
#include <kernel/process.h>
#include <cpu/timer.h>
#include <gui/gnrtx.h>
#include <minilib.h>
#include <system.h>

/*************************
 *                      *
 *   string functions   *
 *                      *
 ************************/

void str_cat(char *s1, char *s2) {
    while (*s1)
        s1++;
    while (*s2)
        *s1++ = *s2++;
    *s1 = '\0';
}

int str_len(char *s) {
    int i = 0;
    while (s[i])
        i++;
    return i;
}

void str_cpy(char *s1, char *s2) {
    int i;
    for (i = 0; s2[i]; i++) {
        s1[i] = s2[i];
    }
    s1[i] = '\0';
}

void  str_ncpy(char *s1, char *s2, int n) {
    int i;
    for (i = 0; i < n && s2[i]; i++) {
        s1[i] = s2[i];
    }
    s1[i] = '\0';
}

void str_reverse(char *s) {
    int i = 0;
    int j = str_len(s) - 1;
    char tmp;
    while (i < j) {
        tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
        i++;
        j--;
    }
}

void int2str(int n, char *s) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) s[i++] = '-';
    s[i] = '\0';

    str_reverse(s);
}

void hex2str(uint32_t n, char *s) {
    int i = 0;
    int tmp;
    char hex[] = "0123456789abcdef";
    do {
        tmp = n % 16;
        s[i++] = hex[tmp];
    } while ((n /= 16) > 0);
    s[i] = 'x';
    s[i+1] = '0';
    s[i+2] = '\0';
    str_reverse(s);
}

int str2int(char *s) {
    int i = 0;
    int n = 0;
    while (s[i] >= '0' && s[i] <= '9') {
        n = 10 * n + (s[i++] - '0');
    }
    return n;
}

int str_cmp(char *s1, char *s2) {
    int i = 0;
    while (s1[i] == s2[i]) {
        if (s1[i] == '\0') return 0;
        i++;
    }
    return s1[i] - s2[i];
}

int str_ncmp(char *s1, char *s2, int n) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (i == n || s1[i] == '\0') return 0;
    }
    if (i == n) return 0;
    return s1[i] - s2[i];
}


/**************************
 *                       *
 *   memory management   *
 *                       *
 *************************/

void mem_move(void *dest, void *source, uint32_t nbytes) {
    if (dest < source) {
        for (uint32_t i = 0; i < nbytes; i++) {
            ((char *) dest)[i] = ((char *) source)[i];
        }
    } else {
        for (int i = nbytes - 1; i >= 0; i--) {
            ((char *) dest)[i] = ((char *) source)[i];
        }
    }
}

void mem_set(void *dest, uint8_t val, uint32_t len) {
    uint8_t *temp = dest;
    for ( ; len != 0; len--)
        *temp++ = val;
}

int mem_cmp(void *s1, void *s2, uint32_t n) {
    uint8_t *p1 = s1;
    uint8_t *p2 = s2;
    for (uint32_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;
}

void free(void *addr) {
    if (addr == NULL)
        return;
    mem_free_addr(addr);
}

void *malloc(uint32_t size) {
    return mem_alloc(size, 0, 1);
}

void *realloc_as_kernel(void *ptr, uint32_t size) {
    void *new_addr = mem_alloc(size, 0, 6);

    if (new_addr == NULL)
        return NULL;

    if (ptr == NULL)
        return new_addr;

    mem_copy(new_addr, ptr, size);
    mem_free_addr(ptr);

    return new_addr;
}

void *calloc(uint32_t size) {
    void *addr = mem_alloc(size, 0, 1);
    if (addr == NULL)
        return NULL;
    mem_set(addr, 0, size);
    return addr;
}

/**************************
 *                       *
 *   user io functions   *
 *                       *
 *************************/

void status_print(int (*func)(), char *verb, char *noun) {
    int old_cursor, new_cursor, status;

    kcprint("[", 0x0F);
    old_cursor = cursor_get_offset();
    kcprint("WORK", 0x0E);
    kcprint("]  ", 0x0F);
    kcprint(verb, 0x07);
    kcprint(" ", 0x0F);
    kcprint(noun, 0x0F);
    kcprint("\n", 0x0F);

    status = func();
    new_cursor = cursor_get_offset();
    cursor_set_offset(old_cursor);

    if (status == 0) {
        kcprint(" OK ", 0x0A);
    } else if (status == 2) {
        kcprint("PASS", 0x0E);
    } else {
        kcprint("FAIL", 0x0C);
    }

    cursor_set_offset(new_cursor);
}

#define LSHIFT  42
#define RSHIFT  54
#define BACK    14
#define ENTER   28
#define RESEND  224
#define SLEEP_T 15
#define FIRST_L 12
#define SC_MAX  57

void kinput(char *buffer, int size) {
    kprint("\e[?25l");

    int sc, last_sc, last_sc_sgt = 0;
    int index = 0;

    for (int i = 0; i < size; i++)
        buffer[i] = '\0';

    int key_ticks = 0;
    int shift = 0;
    char c;

    sc = last_sc = 0;
    while (sc != ENTER) {
        process_sleep(process_get_pid(), SLEEP_T);
        sc = kb_get_scfh();

        if (sc == RESEND || sc == 0) {
            sc = last_sc_sgt;
        } else {
            last_sc_sgt = sc;
        }

        key_ticks = (sc != last_sc) ? 0 : key_ticks + 1;
        last_sc = sc;

        if ((key_ticks < FIRST_L && key_ticks) || key_ticks % 2) {
            continue;
        } else if (sc == LSHIFT || sc == RSHIFT) {
            shift = 1;
        } else if (sc == LSHIFT + 128 || sc == RSHIFT + 128) {
            shift = 0;
        } else if (sc == BACK) {
            if (index == 0) continue;
            buffer[index] = '\0';
            index--;
            kprint("\e[1D \e[1D");
        } else if (sc <= SC_MAX) {
            if (size < index + 2) continue;
            c = kb_sc_to_char(sc, shift);
            if (c == '\0') continue;
            kcnprint(&c, 1, c_blue);
            buffer[index] = c;
            index++;
        }
    }

    buffer[index] = '\0';
    kprint("\e[?25h");
}

void kprintf_va2buf(char *char_buffer, char *fmt, va_list args) {
    int output, buffer_i, i;
    output = buffer_i = i = 0;
    char s[12];

    if ((uint32_t) char_buffer < 2) {
        output = (int) char_buffer + 1;
        char_buffer = sys_safe_buffer;
    }

    while (fmt[i]) {
        if (fmt[i] == '%') {
            i++;
            if (fmt[i] == 's') {
                char *tmp = va_arg(args, char *);
                for (int j = 0; tmp[j]; j++) {
                    char_buffer[buffer_i] = tmp[j];
                    buffer_i++;
                }
            } else if (fmt[i] == 'c') {
                char_buffer[buffer_i] = va_arg(args, int);
                buffer_i++;
            } else if (fmt[i] == 'd') {
                int2str(va_arg(args, int), s);
                for (int j = 0; s[j]; j++) {
                    char_buffer[buffer_i] = s[j];
                    buffer_i++;
                }
            } else if (fmt[i] == 'x') {
                hex2str(va_arg(args, int), s);
                for (int j = 0; s[j]; j++) {
                    char_buffer[buffer_i] = s[j];
                    buffer_i++;
                }
            } else if (fmt[i] == '%') {
                char_buffer[buffer_i] = '%';
                buffer_i++;
            }
        } else {
            char_buffer[buffer_i] = fmt[i];
            buffer_i++;
        }
        i++;
    }
    char_buffer[buffer_i] = '\0';
    if (output == 1) {
        kprint(char_buffer);
    } else if (output == 2) {
        serial_write(SERIAL_PORT_A, char_buffer, buffer_i);
    }
}

void kprintf_buf(char *char_buffer, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kprintf_va2buf(char_buffer, fmt, args);
    va_end(args);
}
