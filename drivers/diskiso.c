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

    for (sector_count = 0; pos[sector_count * 128] != magic; sector_count++);
    return sector_count;
}

int init_diskiso() {
    diskiso_size = 0;

    if (!mboot_get(6)) {
        return 0;
    }

    uint32_t sector_count = analyze_sectors((uint32_t *) GRUBMOD_START);

    kprintf("        | %d sectors found!\n", sector_count);

    uint32_t pos = MEM_BASE_ADDR + PARTS_COUNT * sizeof(allocated_part_t) + 1;
    diskiso_start = pos; 

    kprintf("        | Copying module to 0x%x\n", pos);

    uint32_t mod_end = GRUBMOD_START + sector_count * 512;
    uint32_t end_pos = pos + sector_count * 512;

    // copy the module to the new position starting from the end
    for (uint32_t i = 0; i <= sector_count * 512; i++) {
        *((uint8_t *) (end_pos - i)) = *((uint8_t *) (mod_end - i));
    }

    diskiso_size = sector_count;

    return 2;   // enabled
}

uint32_t diskiso_get_size() {
    return diskiso_size;
}

uint32_t diskiso_get_start() {
    return diskiso_start;
}

void diskiso_read(uint32_t sector, uint32_t *data) {
    if (sector >= diskiso_size) {
        return;
    }

    uint32_t pos = diskiso_start + sector * 512;

    for (int i = 0; i < 128; i++) {
        data[i] = *((uint32_t *) pos);
        pos += 4;
    }
}

void diskiso_write(uint32_t sector, uint32_t *data) {
    if (sector >= diskiso_size) {
        return;
    }

    uint32_t pos = diskiso_start + sector * 512;

    for (int i = 0; i < 128; i++) {
        *((uint32_t *) pos) = data[i];
        pos += 4;
    }
}
