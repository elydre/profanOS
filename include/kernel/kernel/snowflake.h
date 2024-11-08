/*****************************************************************************\
|   === snowflake.h : 2024 ===                                                |
|                                                                             |
|    Kernel memory allocator header                                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef SNOWFLAKE_H
#define SNOWFLAKE_H

#include <ktype.h>

// SNOWFLAKE physical memory manager

typedef struct allocated_part_t {
    uint32_t next; // list index
    uint32_t size;
    uint32_t addr;
    uint8_t  state;
    void    *dir;
    int      pid;
} allocated_part_t;

#define PARTS_COUNT 100  // initial size
#define GROW_SIZE   20   // increase size

/*********************
 *      states      *
 * 0: free          *
 * 1: simple alloc  *
 * 2: initial block *
 * 3: mm struct     *
 * 4: scuba page    *
 * 5: loaded lib    *
 * 6: as kernel     *
*********************/

int mem_init(void);

void *mem_alloc_dir(uint32_t size, uint32_t allign, int pid, void *dir);
void *mem_alloc(uint32_t size, uint32_t allign, int state);

int mem_free_all(uint32_t pid);
int mem_free_addr(void *addr);
int mem_free_dir(void *dir);

/****************************************
 *       MEM_GET_INFO  GET_MODES       *
 * 00: physical memory size            *
 * 01: base address                    *
 * 02: MEM_PARTS size                  *
 * 03: MEM_PARTS addr                  *
 * 04: total alloc count               *
 * 05: total free count                *
 * 06: used memory size                *
 * 07: simple alloc count of pid `arg` *
 * 08: simple alloc size of pid `arg`  *
 * 09: alloc count of type `arg`       *
 * 10: alloc size of type `arg`        *
 * 11: all alloc count of pid `arg`    *
 * 12: all alloc size of pid `arg`     *
****************************************/

uint32_t mem_get_info(char get_mode, int arg);
uint32_t mem_get_alloc_size(void *addr);

#endif
