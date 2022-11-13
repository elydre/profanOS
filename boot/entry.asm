; yuuOS kernel entry edited for profanOS

[extern kernel_main]
[global loader]

KERNEL_STACK_SIZE equ 4096      ; size of stack in bytes

FLAGS equ 0b111
MAGIC equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text                   ; start of the text (code) section
align 4
loader:
    push ebx                    ; keep the multiboot info pointer
    cli                         ; disable interrupts
    call kernel_main            ; load profanOS kernel
