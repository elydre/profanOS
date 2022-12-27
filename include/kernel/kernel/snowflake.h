#ifndef MEM_H
#define MEM_H

#include <type.h>


// SNOWFLAKE memory manager

typedef struct allocated_part_t {
    uint32_t addr;
    uint32_t size;
    int task_id;
    uint8_t state;
    int next; // list index
} allocated_part_t;

#define PARTS_COUNT 50  // initial size
#define GROW_SIZE   10  // increase size

/*********************
 *      states      *
 * 0: free          *
 * 1: simple alloc  *
 * 2: initial block *
 * 3: mm struct     *
 * 4: bin run       *
 * 5: loaded lib    *
*********************/

int mem_init();

uint32_t mem_get_alloc_size(uint32_t addr);
uint32_t mem_alloc(uint32_t size, int state);
int mem_free_addr(uint32_t addr);

void mem_free_all(int task_id);


/************************************
 *     MEM_GET_INFO  GET_MODES     *
 * 00: physical memory size        *
 * 01: base address                *
 * 02: MEM_PARTS size              *
 * 03: MEM_PARTS addr              *
 * 04: total alloc count           *
 * 05: total free count            *
 * 06: used memory size            *
 * 07: ca count by task *get_arg*  *
 * 08: ca size by task *get_arg*   *
 * 09: ca count for bin_run        *
 * 10: ca size for bin_run         *
 * 11: ca count for loaded libs    *
 * 12: ca size for loaded libs     *
************************************/

int mem_get_info(char get_mode, int get_arg);

#endif
