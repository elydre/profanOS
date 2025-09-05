/*****************************************************************************\
|   === isr.c : 2024 ===                                                      |
|                                                                             |
|    Kernel ISR (Interrupt Service Routine)                        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <cpu/ports.h>
#include <cpu/timer.h>
#include <cpu/idt.h>
#include <cpu/isr.h>
#include <minilib.h>
#include <system.h>

extern void syscall_handler(registers_t *r);
extern void mod_syscall(registers_t *r);

interrupt_handler_t interrupt_handlers[256];

int isr_install(void) {
    set_idt_gate(0,  isr0);
    set_idt_gate(1,  isr1);
    set_idt_gate(2,  isr2);
    set_idt_gate(3,  isr3);
    set_idt_gate(4,  isr4);
    set_idt_gate(5,  isr5);
    set_idt_gate(6,  isr6);
    set_idt_gate(7,  isr7);
    set_idt_gate(8,  isr8);
    set_idt_gate(9,  isr9);
    set_idt_gate(10, isr10);
    set_idt_gate(11, isr11);
    set_idt_gate(12, isr12);
    set_idt_gate(13, isr13);
    set_idt_gate(14, isr14);
    set_idt_gate(15, isr15);
    set_idt_gate(16, isr16);
    set_idt_gate(17, isr17);
    set_idt_gate(18, isr18);
    set_idt_gate(19, isr19);
    set_idt_gate(20, isr20);
    set_idt_gate(21, isr21);
    set_idt_gate(22, isr22);
    set_idt_gate(23, isr23);
    set_idt_gate(24, isr24);
    set_idt_gate(25, isr25);
    set_idt_gate(26, isr26);
    set_idt_gate(27, isr27);
    set_idt_gate(28, isr28);
    set_idt_gate(29, isr29);
    set_idt_gate(30, isr30);
    set_idt_gate(31, isr31);

    // remap the PIC
    port_write8(0x20, 0x11);
    port_write8(0xA0, 0x11);
    port_write8(0x21, 0x20);
    port_write8(0xA1, 0x28);
    port_write8(0x21, 0x04);
    port_write8(0xA1, 0x02);
    port_write8(0x21, 0x01);
    port_write8(0xA1, 0x01);
    port_write8(0x21, 0x0);
    port_write8(0xA1, 0x0);

    // install the IRQs
    set_idt_gate(32, irq0);
    set_idt_gate(33, irq1);
    set_idt_gate(34, irq2);
    set_idt_gate(35, irq3);
    set_idt_gate(36, irq4);
    set_idt_gate(37, irq5);
    set_idt_gate(38, irq6);
    set_idt_gate(39, irq7);
    set_idt_gate(40, irq8);
    set_idt_gate(41, irq9);
    set_idt_gate(42, irq10);
    set_idt_gate(43, irq11);
    set_idt_gate(44, irq12);
    set_idt_gate(45, irq13);
    set_idt_gate(46, irq14);
    set_idt_gate(47, irq15);

    // install the syscall interrupt
    set_idt_gate(128, isr128);

    set_idt(); // load with ASM

    return 0;
}

void isr_handler(registers_t *r) {
    r->int_no = r->int_no & 0xFF;

    int need_unlook = 0;

    if (IN_KERNEL) {
        // understandable error for display
        if (r->int_no == 128) {
            sys_fatal("syscall from kernel mode (%x)", r->eax);
        }
    } else {
        sys_entry_kernel();
        need_unlook = 1;
    }

    if (r->int_no == 128) {
        if (r->eax & 0xFF000000)
            mod_syscall(r);
        else
            syscall_handler(r);
    } else {
        sys_interrupt(r->int_no, r->err_code);
    }

    if (need_unlook)
        sys_exit_kernel(0);
}

void irq_handler(registers_t *r) {
    if (r->int_no == 32) {
        TIMER_TICKS++;
        port_write8(0x20, 0x20);
        if (IN_KERNEL)
            return;
    }

    sys_entry_kernel();

    interrupt_handler_t handler = interrupt_handlers[r->int_no];

    if (handler != NULL)
        handler(r);

    sys_exit_kernel(r->int_no == 32 ? 0 : r->int_no + 1);
}

void interrupt_register_handler(uint8_t n, interrupt_handler_t handler) {
    interrupt_handlers[n] = handler;
}

int irq_install(void) {
    // durring the kernel only IRQ0 is enabled (timer)
    port_write8(0x21, 0xFE);
    port_write8(0xA1, 0xFF);

    asm volatile("sti");

    return 0;
}
