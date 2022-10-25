#include <driver/serial.h>
#include <driver/screen.h>
#include <driver/ata.h>
#include <function.h>
#include <string.h>
#include <iolib.h>

#define RAMDISK_ADDR 0x1000000
#define UINT32_PER_SECTOR 128


/* add in this list the paths 
to load in ramdisk at boot */

char * path_to_load[] = {
    "/bin/commands",
    "/bin/shell.bin",
};


/************************
 * READ & WRITE SECTOR *
************************/

void ramdisk_write_sector(uint32_t sector, uint32_t* buffer) {
    uint32_t* ramdisk = (uint32_t*)RAMDISK_ADDR;
    for (uint32_t i = 0; i < UINT32_PER_SECTOR; i++) {
        ramdisk[sector * UINT32_PER_SECTOR + i] = buffer[i];
    }
}

void ramdisk_read_sector(uint32_t LBA, uint32_t out[]) {
    uint32_t* ramdisk = (uint32_t*)RAMDISK_ADDR;
    for (uint32_t i = 0; i < UINT32_PER_SECTOR; i++) {
        out[i] = ramdisk[LBA * UINT32_PER_SECTOR + i];
    } if (out[0] == 0) {
        ata_read_sector(LBA, out);
        ramdisk_write_sector(LBA, out);
    }
}

int sector_is_loaded(uint32_t LBA) {
    uint32_t* ramdisk = (uint32_t*)RAMDISK_ADDR;
    return ramdisk[LBA * UINT32_PER_SECTOR] != 0;
}

/***************************
 * RAMDISK INITIALIZATION *
***************************/

void ramdisk_check_dir(char parent_name[], uint32_t sector_id);

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

void load_file(uint32_t first_sector_id) {
    uint32_t sector[UINT32_PER_SECTOR];
    uint32_t sector_id = first_sector_id;
    do {
        ata_read_sector(sector_id, sector);
        ramdisk_write_sector(sector_id, sector);
        sector_id = sector[UINT32_PER_SECTOR - 1];
    } while (sector[UINT32_PER_SECTOR - 1] != 0);
}

void ramdisk_check_dir(char parent_name[], uint32_t sector_id) {
    if (sector_is_loaded(sector_id)) return;
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
        for (int i = 0; i < ARYLEN(path_to_load); i++) {
            if (str_in_str(fullname, path_to_load[i]) || str_cmp(fullname, path_to_load[i]) == 0) {
                serial_debug("RD-LF", fullname);
                fskprint("load %s", fullname);
                clean_line();
                load_file(sector_id);
            }
        }
        return;
    }
    // check if the path is the beginning of a path to load
    for (int i = 0; i < ARYLEN(path_to_load); i++) {
        if (str_in_str(fullname, path_to_load[i]) || str_in_str(path_to_load[i], fullname) || str_cmp(fullname, path_to_load[i]) == 0) {
            serial_debug("RD-CD", fullname);
            ramdisk_write_sector(sector_id, sector);
            for (int i = 21; i < UINT32_PER_SECTOR - 1; i++) {
                if (sector[i] == 0) continue;
                ramdisk_check_dir(fullname, sector[i]);
            }
        }
    }
}
