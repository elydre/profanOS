#include <cpu/gdt.h>

// GDT: global descriptor table

struct gdt_entry gdt[3];
struct gdt_ptr gdt_p;

void gdt_flush() {
    asm volatile("lgdtl (%0)" : : "r" (&gdt_p));
    asm volatile("mov $0x10, %ax");
    asm volatile("mov %ax, %ds");
    asm volatile("mov %ax, %es");
    asm volatile("mov %ax, %fs");
    asm volatile("mov %ax, %gs");
    asm volatile("mov %ax, %ss");
    asm volatile("ljmp $0x08, $next");
    asm volatile("next:");
}

void gdt_init_entry(int index, unsigned long base, unsigned long limit, unsigned char access, unsigned char granularity) {
    gdt[index].base_low = (base & 0xFFFF);
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].base_high = (base >> 24) & 0xFF;
    gdt[index].limit_low = (limit & 0xFFFF);
    gdt[index].granularity = ((limit >> 16) & 0x0F);
    gdt[index].granularity |= (granularity & 0xF0);
    gdt[index].access = access;
}

void gdt_init() {
    gdt_p.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gdt_p.base = (unsigned int) gdt;

    gdt_init_entry(0, 0, 0, 0, 0);                  // null
    gdt_init_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);   // code 
    gdt_init_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);   // data

    gdt_flush();    
}
