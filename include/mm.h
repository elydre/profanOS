#ifndef MM
#define MM

typedef struct allocated_part_t {
    uint32_t addr;
    uint32_t size;
    int task_id;
    uint8_t state;
    int next; // list index
} allocated_part_t;

#define PARTS_COUNT 10000
#define BASE_ADDR 0x200000

// states:
// 0 - free
// 1 - allocated

void mm_init();

uint32_t mm_alloc(uint32_t size);
int mm_free(uint32_t addr);
void mm_print();

#endif
