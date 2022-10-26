#include <driver/serial.h>
#include <driver/screen.h>
#include <driver/ata.h>
#include <function.h>
#include <system.h>
#include <string.h>
#include <iolib.h>
#include <mem.h>

#define UINT32_PER_SECTOR 128
#define VIRTUAL_DISK_SECTOR 1024

uint32_t *RAMDISK;
int disk_working;

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
    for (uint32_t i = 0; i < UINT32_PER_SECTOR; i++) {
        RAMDISK[sector * UINT32_PER_SECTOR + i] = buffer[i];
    }
    RAMDISK[sector * UINT32_PER_SECTOR] = buffer[0] + 1;
}

int ramdisk_does_sector_isloaded(uint32_t sector) {
    return RAMDISK[sector * UINT32_PER_SECTOR] != 0;
}

void ramdisk_read_sector(uint32_t LBA, uint32_t out[]) {
    if (disk_working && !ramdisk_does_sector_isloaded(LBA)) {
        ata_read_sector(LBA, out);
        ramdisk_write_sector(LBA, out);
        return;
    }
    for (uint32_t i = 0; i < UINT32_PER_SECTOR; i++) {
        out[i] = RAMDISK[LBA * UINT32_PER_SECTOR + i];
    }
    out[0]--;
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
    disk_working = ata_get_status();
    if (!disk_working) sys_warning("ATA disk not found");
    int disk_size = (disk_working ? ata_get_sectors_count() : VIRTUAL_DISK_SECTOR) * UINT32_PER_SECTOR * 4;
    RAMDISK = (uint32_t*)(mem_get_phys_size() - disk_size);
    if (RAMDISK < (uint32_t*) 0x700000) sys_fatal("No enough memory for ramdisk");
    if (disk_working) {
        char path[256];
        for (int i = 0; i < 256; i++) path[i] = 0;
        ramdisk_check_dir(path, 0);
        clean_line();
    }
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
    if (ramdisk_does_sector_isloaded(sector_id)) return;
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

/***************
 * GET & INFO *
***************/

uint32_t get_ramdisk_address() {
    return (uint32_t) RAMDISK;
}
