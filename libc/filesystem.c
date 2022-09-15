#include <filesystem.h>
#include <driver/ata.h>
#include <string.h>
#include <iolib.h>
#include <mem.h>

void parse_path(char path[1000], string_20_t liste_path[]);

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
        suite = i_free_file_and_get_next(suite);
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

void i_add_item_to_dir(uint32_t file_id, uint32_t folder_id) {
    uint32_t dossier[128];
    for (int i = 0; i < 128; i++) dossier[i] = 0;
    read_sectors_ATA_PIO(folder_id, dossier);
    if (!(dossier[0] & 0x8000)) {
        fskprint("Erreur, le secteur %d est vide", folder_id);
        return;
    }
    if (!(dossier[0] & 0x4000)) {
        fskprint("Erreur, le secteur %d n'est pas un dossier", folder_id);
        return;
    }
    int full = 1;
    for (int i = 21; i<128; i++) {
        if (!dossier[i]) {
            dossier[i] = file_id;
            full = 0;
            break;
        }
    }
    if (full) {
        fskprint("Erreur, le dossier est plein");
        return;
    }
    write_sectors_ATA_PIO(folder_id, dossier);
}

void i_get_dir_content(uint32_t id, string_20_t list_name[], int liste_id[]) {
    for (int i = 0; i < 128; i++) list_name[i].name[0] = '\0';
    for (int i = 0; i < 128; i++) liste_id[i] = 0;
    int pointeur_noms = 0;
    int pointeur_liste_id = 0;
    
    uint32_t folder[128];
    read_sectors_ATA_PIO(id, folder);
    if (!(folder[0] & 0x8000)) {
        fskprint("Erreur, le secteur %d est vide", id);
        return;
    }
    if (!(folder[0] & 0x4000)) {
        fskprint("Erreur, le secteur %d n'est pas un dossier", id);
        return;
    }
    uint32_t liste_contenu[108];
    for (int i = 0; i < 108; i++) liste_contenu[i] = 0;
    for (int i = 21; i < 128; i++) {
        liste_contenu[i-21] = folder[i];
    }
    uint32_t content[128];
    uint32_t liste_chars[20];
    char nom[20];
    for (int i = 0; i < 108; i++) {
        if (liste_contenu[i]) {
            read_sectors_ATA_PIO(liste_contenu[i], content);
            for (int j = 0; j < 20; j++) liste_chars[j] = content[j+1];
            for (int j = 0; j < 20; j++) nom[j] = (char) liste_chars[j];
            for (int c = 0; c < 20; c++) list_name[pointeur_noms].name[c] = nom[c];
            liste_id[pointeur_liste_id] = liste_contenu[i];
            pointeur_noms++; pointeur_liste_id++;
        }
    }
}

uint32_t i_path_to_id(char input_path[]) {
    if (strlen(input_path) > 1000) {
        fskprint("Erreur, le chemin d'acces est trop long !");
        return;
    }
    // sanitize path
    char path[1000];
    for (int i=0;i<1000;i++) {path[i]=0;}
    for (int i=0; i<strlen(input_path)+1; i++) {path[i] = input_path[i];}

    if (strcmp("/", path) == 0) {
        return 0;
    }
    
    int path_len = strlen(path);
    fskprint("Longueur du path : %d\n", path_len);
    string_20_t * liste_path = malloc(1000*sizeof(string_20_t));
    parse_path(path, liste_path);


    for (int i = 0; i<count_string(path, '/')+1; i++) {
        fskprint("liste_path[%d] : %s\n", i, liste_path[i].name);
    }

    // NE PAS OUBLIER DE FREE liste_path
}

void parse_path(char path[1000], string_20_t liste_path[]) {
    int index = 0;
    char path_original[1000]; for (int i=0;i<1000;i++) {path_original[i]=0;} ; for (int i=0;i<1000;i++) {path_original[i]=path[i];}
    char path_original_original[1000]; for (int i=0;i<1000;i++) {path_original_original[i]=0;} ; for (int i=0;i<1000;i++) {path_original_original[i]=path[i];}
    char path_left[20]; for (int i=0;i<20;i++) {path_left[i]=path[i];} while(in_string(path_left, '/')) {str_end_split(path_left, '/');}
    
    while (strcmp(path_left, path) != 0) {
        for (int i=0; i<1000; i++) {path[i] = path_original[i];}
        str_start_split(path, '/');
        char string_name[20]; for (int i=0;i<20;i++) {string_name[i]=0;}
        for (int i=0; i<(strlen(path)); i++) {string_name[i] = path_original[i];}
        for (int i=0; i<(1000-strlen(path));i++){path_original[i] = path_original[i+strlen(path)+1];}
        for (int i = 0; i<20; i++) {liste_path[index].name[i] = string_name[i];}
        index++;
    }
    for (int i = 0; i<20; i++) {liste_path[0].name[i] = 0;}
    for (int i=0;i<1000;i++) {path[i]=path_original_original[i];}
}