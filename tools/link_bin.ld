/*****************************************************************************\
|   === link_bin.ld : 2024 ===                                                |
|                                                                             |
|    Linker script for profanOS binaries                           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

ENTRY(_start)

PHDRS {
    text PT_LOAD FLAGS(5);    /* R + X */
    data PT_LOAD FLAGS(6);    /* R + W */
}

SECTIONS {
    . = 0xB0001000;

    .text : {
        *(.text)
        *(.text.*)
    } :text

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } :text

    .data : {
        *(.data)
        *(.data.*)
    } :data

    .bss : {
        *(.bss)
        *(.bss.*)
    } :data
}
