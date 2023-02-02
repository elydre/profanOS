#include <kernel/multiboot.h>
#include <cpu/ports.h>
#include <minilib.h>

// read the initrd from grub module and store it in a buffer

#define MAX_UINT32 (0xFFFFFFFF - 2)

void check_sectors(uint32_t *pos) {
    uint32_t sector[128];
    for (int i = 0;; i++) {
        ata_read_sector(i, sector);
        for (int j = 0; j < 128; j++) {
            if (sector[j] != pos[i * 128 + j]) {
                kprintf("Found %d correct sectors at position %d\n", i, pos);
                return;
            }
        }

    }
}

void initrd_iso() {
    for (int i = 0; i < 32; i++) {
        kprintf("Multiboot data %d: %d\n", i, mboot_get(i));
    }
    uint32_t sector[128];
    ata_read_sector(0, sector);
    // convert sector to char array
    uint32_t magic = sector[0];
    kprintf("Magic: %d\n", magic);
    kprintf("Magic + 1: %d\n", sector[1]);

    for (int i = 0; i < MAX_UINT32; i++) {
        for (int j = 0; j < 128; j++) {
            if (*(uint32_t *) (i + j * 4) != sector[j]) {
                break;
            }
            if (j == 127) {
                kprintf("Found magic at %d\n", i);
                check_sectors((uint32_t *) i);
            }
        }
    }
}
