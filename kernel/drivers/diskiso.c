#include <kernel/snowflake.h>
#include <kernel/multiboot.h>
#include <minilib.h>
#include <system.h>

uint32_t diskiso_start;
uint32_t diskiso_size;

int analyze_sectors(uint32_t *pos) {
    // magic number is 'TITE' for This Is The End
    uint32_t magic = 'T' | ('I' << 8) | ('T' << 16) | ('E' << 24);
    int sector_count;

    for (sector_count = 0; pos[sector_count * 64] != magic; sector_count++);
    return sector_count;
}

int init_diskiso() {
    diskiso_size = 0;

    if (!mboot_get(6)) {
        return 0;
    }

    uint32_t sector_count = analyze_sectors((uint32_t *) GRUBMOD_START);

    uint32_t pos = MEM_BASE_ADDR + PARTS_COUNT * sizeof(allocated_part_t) + 1;
    diskiso_start = pos;

    uint32_t mod_end = GRUBMOD_START + sector_count * 256;
    uint32_t end_pos = pos + sector_count * 256;

    // copy the module to the new position starting from the end
    for (uint32_t i = 0; i <= sector_count * 256; i++) {
        *((uint8_t *) (end_pos - i)) = *((uint8_t *) (mod_end - i));
    }

    diskiso_size = sector_count;

    kprintf("diskiso: %d sectors\n", diskiso_size);

    return 2;   // enabled
}

uint32_t diskiso_get_size() {
    return diskiso_size * 256;
}

uint32_t diskiso_get_start() {
    return diskiso_start;
}

void diskiso_free() {
    diskiso_size = 0;
    free((void *) diskiso_start);
}
