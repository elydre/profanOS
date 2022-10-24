#include <driver/serial.h>
#include <driver/screen.h>
#include <driver/ata.h>
#include <function.h>
#include <string.h>
#include <iolib.h>

#define RAMDISK_ADDR 0x1000000
#define UINT32_PER_SECTOR 128

char * path_to_load[] = {
    "/bin/commands",
};

void load_sector(uint32_t sector, uint32_t* buffer);
void ramdisk_check_dir(char parent_name[], uint32_t sector_id);

uint32_t find_end_of_writed_sector() {
    uint32_t sector[UINT32_PER_SECTOR];
    int min = 0;
    int max = ata_get_sectors_count();
    int mid = (min + max) / 2;
    while (min < max) {
        ata_read_sector(mid, sector);
        if (sector[0] == 0) max = mid;
        else min = mid + 1;
        mid = (min + max) / 2;
    }
    return mid;
}

void clean_line() {
    for (int i = 0; i < 40; i++) kprint(" ");
    kprint("\r");
}

void ramdisk_init() {
    char path[256];
    for (int i = 0; i < 256; i++) path[i] = 0;
    ramdisk_check_dir(path, 0);
    clean_line();
}

void load_sector(uint32_t sector, uint32_t* buffer) {
    uint32_t* ramdisk = (uint32_t*)RAMDISK_ADDR;
    for (uint32_t i = 0; i < UINT32_PER_SECTOR; i++) {
        ramdisk[sector * UINT32_PER_SECTOR + i] = buffer[i];
    }
}

void load_file(uint32_t first_sector_id) {
    uint32_t sector[UINT32_PER_SECTOR];
    uint32_t sector_id = first_sector_id;
    do {
        ata_read_sector(sector_id, sector);
        load_sector(sector_id, sector);
        sector_id = sector[UINT32_PER_SECTOR - 1];
    } while (sector[UINT32_PER_SECTOR - 1] != 0);
}

void ramdisk_check_dir(char parent_name[], uint32_t sector_id) {
    char fullname[256];
    for (int i = 0; i < 256; i++) fullname[i] = parent_name[i];
    uint32_t sector[UINT32_PER_SECTOR];
    ata_read_sector(sector_id, sector);
    if (sector[0] != 0xc000 && sector[0] != 0xa000) {
        sys_fatal("dametokosita find in dir"); 
    }
    char name[20];
    for (int i = 0; i < 20; i++) name[i] = sector[i + 1];
    name[20] = 0;
    if (fullname[1] != 0 && fullname[0] != 0) {
        str_append(fullname, '/');
    }
    str_cat(fullname, name);
    if (sector[0] == 0xa000) {
        fskprint("load %s", fullname);
        clean_line();
        serial_debug("RD-LOAD", fullname);
        load_file(sector_id);
        return;
    }
    // verifer si le chemin est le debut d'un chemin a charger
    load_sector(sector_id, sector);
    for (int i = 0; i < ARYLEN(path_to_load); i++) {
        for (int j = 0; fullname[j] != 0 && path_to_load[i][j] != 0; j++) {
            if (fullname[j] != path_to_load[i][j]) break;
            if (fullname[j + 1] == 0 || path_to_load[i][j + 1] == 0) {
                for (int i = 21; i < UINT32_PER_SECTOR - 1; i++) {
                    if (sector[i] == 0) continue;
                    ramdisk_check_dir(fullname, sector[i]);
                }
                break;
            }
        }
    }
}


void ramdisk_read_sector(uint32_t LBA, uint32_t out[]) {
    uint32_t* ramdisk = (uint32_t*)RAMDISK_ADDR;
    for (uint32_t i = 0; i < UINT32_PER_SECTOR; i++) {
        out[i] = ramdisk[LBA * UINT32_PER_SECTOR + i];
    } if (out[0] == 0) ata_read_sector(LBA, out);
}

void ramdisk_write_sector(uint32_t LBA, uint32_t bytes[]) {
    uint32_t* ramdisk = (uint32_t*)RAMDISK_ADDR;
    for (uint32_t i = 0; i < UINT32_PER_SECTOR; i++) {
        ramdisk[LBA * UINT32_PER_SECTOR + i] = bytes[i];
    }
}
