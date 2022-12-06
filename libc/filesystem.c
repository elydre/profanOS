#include <libc/filesystem.h>
#include <libc/ramdisk.h>
#include <string.h>
#include <system.h>
#include <mem.h>

uint32_t fs_path_to_id(char input_path[], int silence);
void i_parse_path(char path[], string_20_t liste_path[]);
uint32_t i_next_free(uint32_t rec);
uint32_t i_creer_dossier(char nom[]);

void filesystem_init() {
    uint32_t folder_racine[128];
    ramdisk_read_sector(0, folder_racine);
    if (!(folder_racine[0] & 0x8000)) {
        uint32_t location = i_next_free(0);
        if (location != 0)
            sys_fatal("There is already some stuff on the disk !");
        i_creer_dossier("/");
    }
}

uint32_t i_next_free(uint32_t rec) {
    uint32_t x = 0;
    uint32_t sector[128];
    for (uint32_t i = 0; i < rec + 1; i++) {
        ramdisk_read_sector(x, sector);
        while (sector[0] & 0x8000){
            x++;
            ramdisk_read_sector(x, sector);
        }
        if (i != rec) x++;
    }
    return x;
}

uint32_t i_creer_dossier(char nom[]) {
    if (str_is_in(nom, '/') && str_cmp(nom, "/"))
        return sys_error("Dir name cannot contain /");

    uint32_t folder_id = i_next_free(0);
    uint32_t list_to_write[128];
    for (int i = 0; i < 128; i++) list_to_write[i] = 0;
    list_to_write[0] = 0xC000; // 0x8000 + 0x4000

    if (str_len(nom) > 20)
        return sys_error("Dir name cannot exceed 20 characters");

    int list_index = 1;
    for (int i = 0; i < str_len(nom); i++) {
        list_to_write[list_index] = (int) nom[i];
        list_index++;
    }

    ramdisk_write_sector(folder_id, list_to_write);
    return folder_id;
}

uint32_t i_creer_index_de_fichier(char nom[]) {
    if (str_is_in(nom, '/'))
        return sys_error("File name cannot contain /");

    if (str_len(nom) > 20) 
        return sys_error("File name cannot exceed 20 characters");

    uint32_t location = i_next_free(0);
    uint32_t location_file = i_next_free(1);

    // write intex
    uint32_t list_to_write[128];
    for (int i = 0; i < 128; i++) list_to_write[i] = 0;
    list_to_write[0] = 0xA000;
    int list_index = 1;
    for (int i = 0; i < str_len(nom); i++) {
        list_to_write[list_index] = (int) nom[i];
        list_index++;
    }
    list_to_write[127] = location_file;
    ramdisk_write_sector(location, list_to_write);

    // write file
    for (int i = 0; i < 128; i++) list_to_write[i] = 0;
    list_to_write[0] = 0x9000;
    list_to_write[127] = 0;
    ramdisk_write_sector(location_file, list_to_write);

    return location;
}

uint32_t i_free_file_and_get_next(uint32_t file_id) {
    uint32_t sector[128];
    ramdisk_read_sector(file_id, sector);
    uint32_t suite = sector[127];

    for (int i = 0; i < 128; i++) sector[i] = 0;
    ramdisk_write_sector(file_id, sector);

    return suite;
}

void i_set_data_to_file(uint32_t data[], uint32_t data_size, uint32_t file_id) {
    uint32_t sector[128];
    ramdisk_read_sector(file_id, sector);

    uint32_t file_index = sector[127];

    if (!(sector[0] & 0xA000)) {
        sys_error("Sector is not a file");
        return;
    }

    uint32_t suite = file_index;

    while (suite) suite = i_free_file_and_get_next(suite);

    for (uint32_t i = 0; i < (data_size / 126 + 1); i++) {
        uint32_t part[128];
        for (int i = 0; i < 128; i++) part[i] = 0;
        int ui = 1;
        part[0] = 0x9000;
        for (int j = 0; j < 126; j++) {
            if (i * 126 + j >= data_size) {
                ui = 0;
                break;
            }
            part[j+1] = data[i*126+j] + 1;
        }
        if (ui) part[127] = i_next_free(1); 
        ramdisk_write_sector(file_index, part);
        file_index = part[127];
    }
}

void i_add_item_to_dir(uint32_t file_id, uint32_t folder_id) {
    uint32_t dossier[128];
    ramdisk_read_sector(folder_id, dossier);

    if (!(dossier[0] & 0x8000)) {
        sys_error("Sector is empty");
        return;
    }
    if (!(dossier[0] & 0x4000)) {
        sys_error("Sector is not a dir");
        return;
    }
    int full = 1;
    for (int i = 21; i < 128; i++) {
        if (!dossier[i]) {
            dossier[i] = file_id;
            full = 0;
            break;
        }
    }
    if (full) {
        sys_error("The dir is full");
        return;
    }
    ramdisk_write_sector(folder_id, dossier);
}

int i_size_folder(uint32_t id_folder) {
    uint32_t folder[128];
    ramdisk_read_sector(id_folder, folder);
    int size = 0;
    for (int i = 21; i < 128; i++) {
        if (folder[i]) size++;
    }
    return size;
}

void i_parse_path(char path[], string_20_t liste_path[]) {
    int index = 0;
    int index_in_str = 0;
    for (int i = 0; i < str_len(path); i++) {
        if (path[i] != '/') {
            liste_path[index].name[index_in_str] = path[i];
            index_in_str++;
        } else {
            liste_path[index].name[index_in_str] = '\0';
            index++;
            index_in_str = 0;
        }
    }
    for (int i = 0; i<20; i++) liste_path[0].name[i] = 0;
}

uint32_t i_get_parent_id(char path[]) {
    if (!str_cmp(path, "/")) {return 0;}
    if (str_count(path, '/') == 1) {return 0;}
    char *new_path = malloc(sizeof(char) * str_len(path));
    for (int i=0; i<str_len(path); i++) new_path[i] = path[i];
    for (int i = str_len(path); i>0; i--) {
        if (new_path[i] == '/') {
            new_path[i] = '\0';
            break;
        }
    }
    free(new_path);
    return fs_path_to_id(new_path, 0);
}

void i_assemble_path(char old[], char new[], char result[]) {
    result[0] = '\0'; str_cpy(result, old);
    if (result[str_len(result) - 1] != '/') str_append(result, '/');
    for (int i = 0; i < str_len(new); i++) str_append(result, new[i]);
}

// PUBLIC FUNCTIONS

uint32_t fs_path_to_id(char input_path[], int silence) {
    //init
    int x = 0;
    int in_folder = 0;
    int start_from_liste_path = 0;

    // sanitize path
    char *path = malloc((str_len(input_path)+1)*sizeof(char));
    for (int i = 0; i < str_len(input_path) + 1; i++) path[i] = input_path[i];

    if (str_cmp("/", path) == 0) {
        free(path);
        return 0;
    }

    string_20_t * liste_path = calloc(str_len(path) * sizeof(string_20_t));
    i_parse_path(path, liste_path);

    int folder_size = i_size_folder(0);
    int taille_path = str_count(path, '/') + 1;
    string_20_t * liste_noms = malloc(folder_size * sizeof(string_20_t));
    for (int i = 0; i < folder_size; i++) liste_noms[i].name[0] = '\0';
    uint32_t *liste_id = malloc(folder_size * sizeof(int));
    for (int i = 0; i < folder_size; i++) liste_id[i] = 0;
    fs_get_dir_content(0, liste_noms, liste_id);

    start_from_liste_path++;
    taille_path--;

    in_folder = 0;
    for (int i = 0; i < folder_size; i++) {
        if (!str_cmp(liste_path[0+start_from_liste_path].name, liste_noms[i].name)) in_folder = 1;
    }
    if (!in_folder) {
        free(liste_path);
        free(liste_noms);
        free(liste_id);
        free(path);
        if (!silence) sys_error("The path does not lead to a dir");
        return -1;
    }

    if (taille_path == 1) {
        x = 0;
        for (int i = 0; i < folder_size; i++) {
            if (!str_cmp(liste_noms[i].name, liste_path[0+start_from_liste_path].name)) break;
            x++;
        }
        free(liste_path);
        free(liste_noms);
        free(liste_id);
        free(path);
        return liste_id[x];
    }

    while (taille_path != 1) {
        x = 0;
        for (int i = 0; i < folder_size; i++) {
            if (!str_cmp(liste_noms[i].name, liste_path[0+start_from_liste_path].name)) break;
            x++;
        }

        uint32_t contenu_path_0 = liste_id[x];

        folder_size = i_size_folder(contenu_path_0);
        free(liste_noms);
        free(liste_id);
        string_20_t * liste_noms = malloc(folder_size * sizeof(string_20_t));
        for (int i = 0; i < folder_size; i++) liste_noms[i].name[0] = '\0';
        uint32_t *liste_id = malloc(folder_size * sizeof(int));
        for (int i = 0; i < folder_size; i++) liste_id[i] = 0;
        fs_get_dir_content(contenu_path_0, liste_noms, liste_id);

        start_from_liste_path++;
        taille_path--;
    }

    in_folder = 0;
    for (int i = 0; i < folder_size; i++) {
        if (!str_cmp(liste_path[start_from_liste_path].name, liste_noms[i].name)) in_folder = 1;
    }
    if (!in_folder) {
        if (!silence) sys_error("The path does not lead to something that exists");
        free(liste_path);
        free(liste_noms);
        free(liste_id);
        free(path);
        return -1;
    }

    x = 0;
    for (int i = 0; i < folder_size; i++) {
        if (!str_cmp(liste_noms[i].name, liste_path[0 + start_from_liste_path].name)) break;
        x++;
    }
    uint32_t contenu_path_0 = liste_id[x];

    free(liste_path);
    free(liste_noms);
    free(liste_id);
    free(path);
    return contenu_path_0;
}

uint32_t fs_get_used_sectors(uint32_t disk_size) {
    uint32_t total = 0;
    for (uint32_t i = 0; i < disk_size; i++) {
        uint32_t sector[128];
        ramdisk_read_sector(i, sector);
        if (sector[0] & 0x8000) total++;
    }
    return total;
}

uint32_t fs_is_disk_full(uint32_t disk_size) {
    return disk_size == fs_get_used_sectors(disk_size);
}

uint32_t fs_make_dir(char path[], char folder_name[]) {
    char *path_to_folder = malloc(0x1000);
    i_assemble_path(path, folder_name, path_to_folder);
    if (fs_does_path_exists(path_to_folder)) {
        free(path_to_folder);
        return sys_error("The path already exists");
    } free(path_to_folder);

    uint32_t dossier = i_creer_dossier(folder_name);
    uint32_t id_to_set = fs_path_to_id(path, 0);
    if ((int) id_to_set != -1) i_add_item_to_dir(dossier, id_to_set);
    else sys_error("The path specified in fs_make_dir does not exist");
    return dossier;
}

uint32_t fs_make_file(char path[], char file_name[]) {
    char *path_to_file = malloc(0x1000);
    i_assemble_path(path, file_name, path_to_file);
    if (fs_does_path_exists(path_to_file)) {
        free(path_to_file);
        return sys_error("The path already exists");
    } free(path_to_file);

    uint32_t fichier_test = i_creer_index_de_fichier(file_name);
    uint32_t id_to_set = fs_path_to_id(path, 0);
    i_add_item_to_dir(fichier_test, id_to_set);
    return fichier_test;
}

void fs_write_in_file(char path[], uint32_t data[], uint32_t data_size) {
    uint32_t id_to_set = fs_path_to_id(path, 0);
    i_set_data_to_file(data, data_size, id_to_set);
}

// Note : ne pas utiliser dans un check d'une boucle sauf si vraiment nessessaire
uint32_t fs_get_file_size(char path[]) {
    uint32_t id_file_index = fs_path_to_id(path, 0);
    uint32_t sector[128];
    ramdisk_read_sector(id_file_index, sector);
    if (!(sector[0] & 0xA000)){
        sys_error("Sector is not a file");
        return -1;
    }
    uint32_t sector_size = 0;
    while (sector[127]) {
        sector_size++;
        id_file_index = sector[127];
        ramdisk_read_sector(id_file_index, sector);
    }
    return sector_size;
}

void *fs_declare_read_array(char path[]) {
    return calloc((fs_get_file_size(path) * sizeof(uint32_t) + 1) * 126);
}

// How to declare data : uint32_t *data = fs_declare_read_array(path);
// How to free data    : free(data);
void fs_read_file(char path[], uint32_t data[]) {
    uint32_t sector[128];
    ramdisk_read_sector(fs_path_to_id(path, 0), sector);
    uint32_t id_file_index = sector[127];
    ramdisk_read_sector(id_file_index, sector);
    if (!(sector[0] & 0xA000))
        sys_error("Sector is not a file");
    uint32_t index = 0;
    ramdisk_read_sector(id_file_index, sector);
    for (int i=1; i < 127; i++) {
        data[index] = sector[i] - 1;
        index++;
    }
    id_file_index = sector[127];
    while (id_file_index) {
        ramdisk_read_sector(id_file_index, sector);
        for (int i = 1; i < 127; i++) {
            data[index] = sector[i] - 1;
            index++;
        }
        id_file_index = sector[127];
    }
}

int fs_does_path_exists(char path[]) {
    return (int) fs_path_to_id(path, 1) != -1;
}

int fs_type_sector(uint32_t id_sector) {
    uint32_t sector[128];
    ramdisk_read_sector(id_sector, sector);
    if (sector[0] == 0x8000) return -1; // shouldn't hstr_append
    if (sector[0] == 0x9000) return 1;  // file content
    if (sector[0] == 0xa000) return 2;  // file index
    if (sector[0] == 0xc000) return 3;  // folder
    return 0;                           // default (sector empty)
}

void fs_get_dir_content(uint32_t id, string_20_t list_name[], uint32_t liste_id[]) {
    for (int i = 0; i < 128; i++) list_name[i].name[0] = '\0';
    for (int i = 0; i < 128; i++) liste_id[i] = 0;
    int pointeur_noms = 0;
    int pointeur_liste_id = 0;
    
    uint32_t folder[128];
    ramdisk_read_sector(id, folder);

    if (!(folder[0] & 0x8000)) {
        sys_error("Sector is empty");
        return;
    }

    if (!(folder[0] & 0x4000)) {
        sys_error("Sector is not a folder");
        return;
    }

    uint32_t liste_contenu[108];
    for (int i = 0; i < 108; i++) liste_contenu[i] = 0;
    for (int i = 21; i < 128; i++) liste_contenu[i-21] = folder[i];

    uint32_t content[128];
    uint32_t liste_chars[20];
    char nom[20];
    for (int i = 0; i < 108; i++) {
        if (liste_contenu[i]) {
            ramdisk_read_sector(liste_contenu[i], content);
            for (int j = 0; j < 20; j++) liste_chars[j] = content[j + 1];
            for (int j = 0; j < 20; j++) nom[j] = (char) liste_chars[j];
            for (int c = 0; c < 20; c++) list_name[pointeur_noms].name[c] = nom[c];
            liste_id[pointeur_liste_id] = liste_contenu[i];
            pointeur_noms++; pointeur_liste_id++;
        }
    }
}

int fs_get_folder_size(char path[]) {
    return i_size_folder(fs_path_to_id(path, 0));
}
