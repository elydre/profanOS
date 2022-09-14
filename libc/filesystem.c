#include <filesystem.h>
#include <driver/ata.h>
#include <string.h>
#include <iolib.h>

void init_filesystem() {
    return;
}

uint32_t i_next_free(uint32_t rec) {
    uint32_t x = 0;
    uint32_t sector[128];
    for (uint32_t i = 0; i < rec + 1; i++) {
        read_sectors_ATA_PIO(x, sector);
        while (sector[0] & 0x8000){
            x++;
            read_sectors_ATA_PIO(x, sector);
        }
        if (i != rec) x++;
    }
    return x;
}

uint32_t i_creer_dossier(char nom[]) {
    uint32_t folder_id = i_next_free(0);
    uint32_t list_to_write[128];
    for (int i = 0; i < 128; i++) list_to_write[i] = 0;
    list_to_write[0] = 0xC000; // 0x8000 + 0x4000

    if (strlen(nom) > 20) {
        fskprint("$3nom trop long, max 20 caracteres, nom actuel: $0%s\n", nom);
        return 0;
    }

    // TODO : check if the name is already taken

    int list_index = 1;
    for (int i = 0; i < strlen(nom); i++) {
        list_to_write[list_index] = (int) nom[i];
        list_index++;
    }

    write_sectors_ATA_PIO(folder_id, list_to_write);
    return folder_id;
}

void i_add_pointeur_to_dossier() {
    fskprint("$3ERROR ID 0 : SHOULD NOT BE REACHABLE");
}

uint32_t i_creer_index_de_fichier(char nom[]) {
    if (strlen(nom) > 20) {
        fskprint("$3nom trop long, max 20 caracteres, nom actuel: $0%s\n", nom);
        return 0;
    }

    uint32_t location = i_next_free(0);
    uint32_t location_file = i_next_free(1);

    // TODO : check if the name is already taken

    // write intex
    uint32_t list_to_write[128];
    for (int i = 0; i < 128; i++) list_to_write[i] = 0;
    list_to_write[0] = 0xA000;
    int list_index = 1;
    for (int i = 0; i < strlen(nom); i++) {
        list_to_write[list_index] = (int) nom[i];
        list_index++;
    }
    list_to_write[127] = location_file;
    write_sectors_ATA_PIO(location, list_to_write);

    // write file
    for (int i = 0; i < 128; i++) list_to_write[i] = 0;
    list_to_write[0] = 0x9000;
    list_to_write[127] = 0;
    write_sectors_ATA_PIO(location_file, list_to_write);

    return location;
}

uint32_t i_free_file_and_get_next(uint32_t file_id) {
    uint32_t sector[128];
    for (int i = 0; i < 128; i++) sector[i] = 0;
    read_sectors_ATA_PIO(file_id, sector);
    uint32_t suite = sector[127];
    for (int i = 0; i < 128; i++) sector[i] = 0;
    write_sectors_ATA_PIO(file_id, sector);
    fskprint("FREE %d\n", suite);
    return suite;
}

// TO FIX : Vide aussi a tord l'index de fichier voir "0x001"
void i_set_data_to_file(char data[], uint32_t data_size, uint32_t file_id) {
    uint32_t sector[128];
    for (int i = 0; i < 128; i++) sector[i] = 0;
    read_sectors_ATA_PIO(file_id, sector);
    uint32_t file_index = sector[127];

    if (!(sector[0] & 0xA000)) {
        fskprint("Erreur, le secteur %d n'est pas un fichier", file_id);
        return;
    }

    uint32_t suite = file_index;

    while (suite) {
        suite = i_free_file_and_get_next(suite); // 0x001
    }

    for (uint32_t i = 0; i < (data_size / 126 + 1); i++) {
        uint32_t part[128];
        for (int i = 0; i < 128; i++) part[i] = 0;
        int ui = 1;
        part[0] = 0x9000;
        for (int j = 0; j < 126; j++) {
            if (i*126+j >= data_size) {
                ui = 0;
                break;
            }
            part[j+1] = data[i*126+j];
        }
        if (ui) part[127] = i_next_free(1); 
        write_sectors_ATA_PIO(file_index, part);
        file_index = part[127];
    }
}
