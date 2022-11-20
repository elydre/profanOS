#include <driver/serial.h>
#include <gui/gnrtx.h>
#include <driver/ata.h>
#include <function.h>
#include <system.h>
#include <string.h>
#include <iolib.h>
#include <mem.h>

#define UINT32_PER_SECTOR 128
#define RAMDISK_SECTOR 2048     // 1Mo
#define RAMDISK_SIZE RAMDISK_SECTOR * UINT32_PER_SECTOR * 4

uint32_t ata_table[RAMDISK_SECTOR];
uint32_t *RAMDISK;
int disk_working;
int table_pos = 0;

/* add in this list the paths 
to load in ramdisk at boot */

char *path_to_load[] = {
    "/bin/commands",
    "/bin/shell.bin",
    "/sys",
};


/************************
 * READ & WRITE SECTOR *
************************/

int ramdisk_sector_internal_pos(uint32_t sector) {
    if (!disk_working) {
        if (sector < RAMDISK_SECTOR) return sector;
    } else {
        for (int i = 0; i < table_pos; i++) {
            if (ata_table[i] == sector) return i;
        }
    }
    return -1;
}

void ramdisk_read_sector(uint32_t LBA, uint32_t out[]) {
    int internal = ramdisk_sector_internal_pos(LBA);
    if (internal == -1) {
        if (!disk_working) sys_error("Sector not found in ramdisk");
        else ata_read_sector(LBA, out);
        return;
    }
    for (int i = 0; i < UINT32_PER_SECTOR; i++) {
        out[i] = RAMDISK[internal * UINT32_PER_SECTOR + i];
    }
}

void ramdisk_load_sector(uint32_t ATA_LBA, uint32_t in[]) {
    if (table_pos >= RAMDISK_SECTOR) {
        sys_fatal("No more space in ramdisk");
    }
    if (ramdisk_sector_internal_pos(ATA_LBA) != -1) return;
    ata_table[table_pos] = ATA_LBA;
    for (int i = 0; i < UINT32_PER_SECTOR; i++) {
        RAMDISK[(table_pos * UINT32_PER_SECTOR) + i] = in[i];
    }
    table_pos++;
}

void ramdisk_write_sector(uint32_t sector, uint32_t* buffer) {
    if (disk_working) {
        ata_write_sector(sector, buffer);
    }
    int internal = ramdisk_sector_internal_pos(sector);
    if (internal == -1) {
        if (!disk_working) {
            sys_error("Sector not found in ramdisk");
        }
        return;
    }
    for (int i = 0; i < UINT32_PER_SECTOR; i++) {
        RAMDISK[internal * UINT32_PER_SECTOR + i] = buffer[i];
    }
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
    RAMDISK = (uint32_t*)(mem_get_phys_size() - RAMDISK_SIZE);

    if (RAMDISK < (uint32_t*) 0x700000)
        sys_fatal("No enough memory for ramdisk");

    disk_working = ata_get_sectors_count() > 0;
    if (disk_working) {
        char path[256];
        for (int i = 0; i < 256; i++) path[i] = 0;
        ramdisk_check_dir(path, 0);
        clean_line();
    } else {
        sys_warning("ATA disk not working");
    }
}

void load_file(uint32_t first_sector_id) {
    uint32_t sector[UINT32_PER_SECTOR];
    uint32_t sector_id = first_sector_id;
    do {
        ata_read_sector(sector_id, sector);
        ramdisk_load_sector(sector_id, sector);
        sector_id = sector[UINT32_PER_SECTOR - 1];
    } while (sector[UINT32_PER_SECTOR - 1] != 0);
}

void ramdisk_check_dir(char parent_name[], uint32_t sector_id) {
    if (ramdisk_sector_internal_pos(sector_id) != -1) return;

    char fullname[256];
    for (int i = 0; i < 256; i++) fullname[i] = parent_name[i];
    uint32_t sector[UINT32_PER_SECTOR];
    ata_read_sector(sector_id, sector);
    if (sector[0] == 0x9000) {
        sys_warning("dir pointed to file content");
        return;
    }
    if (sector[0] != 0xc000 && sector[0] != 0xa000) {
        fskprint("FATAL: %x in sec %d\n", sector[0], sector_id);
        sys_fatal("dametokosita find in dir");
    }

    char name[20];
    for (int i = 0; i < 20; i++) name[i] = sector[i + 1];
    name[20] = 0;
    if (fullname[1] != 0 && fullname[0] != 0) str_append(fullname, '/');
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
            // serial_debug("RD-CD", fullname);
            ramdisk_load_sector(sector_id, sector);
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

uint32_t ramdisk_get_address() {
    return (uint32_t) RAMDISK;
}

uint32_t ramdisk_get_size() {
    return RAMDISK_SECTOR;
}

uint32_t ramdisk_get_used() {
    return table_pos;
}

