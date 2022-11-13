; yuuOS kernel entry edited for profanOS

[extern kernel_main]
[global loader]

KERNEL_STACK_SIZE equ 4096          ; size of stack in bytes

FLAGS equ 0b111
MAGIC equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text                       ; start of the text (code) section
align 4                             ; the code must be 4 byte aligned
mov esp, kernel_stack + KERNEL_STACK_SIZE 
loader:
    call kernel_main            ; load our kernel
    
section .bss
align 4                         ; align at 4 bytes
kernel_stack:                   ; label points to beginning of memory
    resb KERNEL_STACK_SIZE      ; reserve stack for the kernel
