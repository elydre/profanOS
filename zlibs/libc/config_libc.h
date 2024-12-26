/*****************************************************************************\
|   === config_libc.h : 2024 ===                                              |
|                                                                             |
|    Global configuration file for profan libc                     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef CONFIG_LIBC_H
#define CONFIG_LIBC_H

#define PROFAN_LIBC_VERSION "2.3 rev 4"

/****** PROFAN EXTRA ***********************************************/

#define NOT_IMPLEMENTED_ABORT 1 // exit on not implemented function
#define _JOIN_LIBCC_INSIDE 1    // create 64-bit division functions


/****** STDIO ******************************************************/

#define STDIO_BUFFER_SIZE 0x1000 // file buffer size (4KB)
#define STDIO_BUFFER_READ 100    // read cache (use file buffer)


/****** STDLIB *****************************************************/

#define SYSTEM_SHELL_PATH "/bin/fatpath/dash.elf" // system() shell


/****** BUDDY ALLOCATOR ********************************************/

#define _BUDDY_DEFAULT_SIZE 16  // 16 pages (64KB)

#define _BUDDY_MDATA_ADDR ((void *) 0xD0000000)  // metadata
#define _BUDDY_ARENA_ADDR ((void *) 0xD1000000)  // block arena

#endif
