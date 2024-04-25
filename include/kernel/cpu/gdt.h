/****** This file is part of profanOS **************************\
|   == gdt.h ==                                      .pi0iq.    |
|                                                   d"  . `'b   |
|   Kernel Global Descriptor Table header           q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#ifndef GDT_H
#define GDT_H

struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

void gdt_init(void);

#endif
