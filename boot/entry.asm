/*****************************************************************************\
|   === entry.asm : 2024 ===                                                  |
|                                                                             |
|    Kernel entry point with multiboot support                     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

[extern kernel_main]
[global loader]

FLAGS equ 0b111
MAGIC equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text               ; start of the text (code) section
align 4
loader:
    push ebx                ; keep the multiboot info pointer
    cli                     ; disable interrupts
    call kernel_main        ; load profanOS kernel
