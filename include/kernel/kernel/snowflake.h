#ifndef SNOWFLAKE_H
#define SNOWFLAKE_H

#include <ktype.h>

// SNOWFLAKE physical memory manager

typedef struct allocated_part_t {
    uint32_t addr;
    uint32_t size;
    int task_id;
    uint8_t state;
    int next; // list index
} allocated_part_t;

#define PARTS_COUNT 100  // initial size
#define GROW_SIZE   20  // increase size

/*********************
 *      states      *
 * 0: free          *
 * 1: simple alloc  *
 * 2: initial block *
 * 3: mm struct     *
 * 4: run stack     *
 * 5: loaded lib    *
 * 6: as kernel     *
 * 7: scuba vpage   *
*********************/

int mem_init(void);

uint32_t mem_get_alloc_size(uint32_t addr);
uint32_t mem_alloc(uint32_t size, uint32_t align, int task_id);
int mem_free_addr(uint32_t addr);

int mem_free_all(int task_id);


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
 * 09: ca count of type *get_arg*  *
 * 10: ca size of type *get_arg*   *
************************************/

int mem_get_info(char get_mode, int get_arg);

#endif
