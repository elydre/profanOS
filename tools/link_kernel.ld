/*****************************************************************************\
|   === link_kernel.ld : 2024 ===                                             |
|                                                                             |
|    Linker script for the kernel                                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

ENTRY(kentry)                       /* entry label name */

SECTIONS {
    . = 0x100000;                   /* the code should be loaded at 1 MB */
    .multiboot ALIGN (0x1000): {    /* align at 4 KB */
        *(.multiboot)               /* the multiboot header */
    }
    .text ALIGN (0x1000): {
        *(.text)                    /* all text sections from all files */
    }
    .rodata ALIGN (0x1000): {
        *(.rodata*)                 /* all read-only data sections from all files */
    }
    .bss ALIGN (0x1000): {
        *(COMMON)                   /* all COMMON sections from all files */
        *(.bss)                     /* all bss sections from all files */
    }
    .data ALIGN (0x1000): {
        *(.data*)                   /* all data sections from all files */
    }
}
