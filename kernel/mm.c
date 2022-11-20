#include <iolib.h>
#include <type.h>
#include <mm.h>


allocated_part_t *MEM_PARTS;
int first_part_index = 0;
int FAA; // first allocable address

void mm_init() {
    MEM_PARTS = (allocated_part_t *) BASE_ADDR;
    for (int i = 0; i < PARTS_COUNT; i++) {
        MEM_PARTS[i].state = 0;
    }

    FAA = BASE_ADDR + sizeof(allocated_part_t) * PARTS_COUNT;

    MEM_PARTS[0].state = 1;
    MEM_PARTS[0].size = 0;
    MEM_PARTS[0].addr = FAA;
    MEM_PARTS[0].next = 1;


    fskprint("memory manager initialized at %x\n", MEM_PARTS);
    fskprint("first available address: %x\n", FAA);
    fskprint("memory manager size: %dKo\n", sizeof(allocated_part_t) * PARTS_COUNT / 1024);
}

int mm_get_unused_index() {
    for (int i = 0; i < PARTS_COUNT; i++) {
        if (MEM_PARTS[i].state == 0) return i;
    }
    fskprint("no more block available\n");
    return -1;
}

void del_occurence(int index) {
    int i = first_part_index;
    while (MEM_PARTS[i].state != 0) {
        if (MEM_PARTS[i].next == index) {
            MEM_PARTS[i].next = mm_get_unused_index();
            return;
        }
        i = MEM_PARTS[i].next;
    }
}

uint32_t mm_alloc(uint32_t size) {
    if (size == 0) return 0;
    // parcours de la liste des parties allouées
    int index, old_index, exit_mode;
    uint32_t last_addr;
    index = first_part_index;

    last_addr = FAA;
    while (1) {
        fskprint("$Eindex: %d, addr: %x, size: %d, state: %d, next: %d\n", index, MEM_PARTS[index].addr, MEM_PARTS[index].size, MEM_PARTS[index].state, MEM_PARTS[index].next);
        // si la partie est libre
        if (MEM_PARTS[index].state == 0) {
            // TODO: verifier si 'last_addr + size' est dans la memoire physique
            exit_mode = 1;
            break;
        }
        if (MEM_PARTS[index].addr - last_addr >= size) {
            // last_addr = MEM_PARTS[index].addr + MEM_PARTS[index].size;
            // on peut allouer la partie ici
            exit_mode = 0;
            break;
        }

        last_addr = MEM_PARTS[index].addr + MEM_PARTS[index].size;

        old_index = index;
        index = MEM_PARTS[index].next;
    }

    fskprint("exit mode: %d\n", exit_mode);

    int new_index = mm_get_unused_index();
    if (new_index == -1) return NULL;

    int i = exit_mode ? index: new_index;

    MEM_PARTS[i].addr = last_addr;
    MEM_PARTS[i].size = size;
    MEM_PARTS[i].task_id = 0; // TODO
    MEM_PARTS[i].state = 1;

    if (exit_mode == 0) {
        del_occurence(new_index);
        fskprint("del occurence(%d)\n", new_index);

        MEM_PARTS[old_index].next = new_index;
        MEM_PARTS[new_index].next = index;
    } else {
        int new_index = mm_get_unused_index();
        if (new_index == -1) return NULL;
        MEM_PARTS[index].next = new_index;
    }

    fskprint("new index: %d, addr: %x, size: %d, state: %d, next: %d\n", i, MEM_PARTS[i].addr, MEM_PARTS[i].size, MEM_PARTS[i].state, MEM_PARTS[i].next);

    return last_addr;
}

int mm_free(uint32_t addr) {
    int index = first_part_index;
    int last_index = -1;
    while (1) {
        if (MEM_PARTS[index].addr == addr && last_index != -1) {
            MEM_PARTS[last_index].next = MEM_PARTS[index].next;
            fskprint("block %d point to %d\n", last_index, MEM_PARTS[last_index].next);
            MEM_PARTS[index].state = 0;
            return 0; // success
        }
        last_index = index;
        index = MEM_PARTS[index].next;
    }
    fskprint("no block found at %x\n", addr);
    return -1; // error
}

void mm_print() {
    int index = first_part_index;
    while (1) {
        fskprint("index: %d, addr: %x, size: %d, state: %d, next: %d\n", index, MEM_PARTS[index].addr, MEM_PARTS[index].size, MEM_PARTS[index].state, MEM_PARTS[index].next);
        if (MEM_PARTS[index].state == 0) break;
        index = MEM_PARTS[index].next;
    }
}