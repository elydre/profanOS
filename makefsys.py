# full code: https://github.com/elydre/elydre/blob/main/projet/profan/file%20system/file-system.py

from math import ceil
import itertools
import os
import sys

__DOCUMENTATION__ = """
0x8000 ==> secteur écrit
0x4000 ==> dossier ici
0x2000 ==> index de fichier ici
0x1000 ==> fichier ici
path = "/dir/.../" + "name"
"""

IS_LINUX = os.name == "posix"
EDIT_FOLDER = "/out/disk"

def getFolderSize(folder):
    total_size = os.path.getsize(folder)
    for item in os.listdir(folder):
        itempath = os.path.join(folder, item)
        if os.path.isfile(itempath):
            total_size += os.path.getsize(itempath)
        elif os.path.isdir(itempath):
            total_size += getFolderSize(itempath)
    return total_size

NUMBER_OF_MO = ceil((getFolderSize(f".{EDIT_FOLDER}")/1000000)*5)
print(f"LE DISQUE VA FAIRE {NUMBER_OF_MO} MEGAOCTETS!")
DISK_SIZE = 2048 * NUMBER_OF_MO
disque = [[0] * 128] * DISK_SIZE

COLOR_INFO = (170, 170, 170)

def cprint(color, text, end="\n"):
    r, g, b = color
    print(f"\033[38;2;{r};{g};{b}m | {text}\033[0m", end=end)

def p_write_sectors_ATA_PIO(secteur, data):
    if secteur < 0:
        p_print_and_exit(f"CONAR TU ECRIT PAS SUR LE SECTEUR {secteur}")
    if len(data) != 128 :
        p_print_and_exit("Erreur: la taille de la donnée doit être de 128 octets")
    disque[secteur] = data
def p_read_sectors_ATA_PIO(secteur):
    try:
        return disque[secteur]
    except IndexError:
        p_print_and_exit(f"CONAR TU LIS PAS SUR LE SECTEUR {secteur}")

def p_print_and_exit(msg):
    cprint(COLOR_INFO, msg)
    sys.exit(1)

def p_disk_to_file():
    cprint(COLOR_INFO, "Dump de disque dans HDD.bin, merci d'attendre")
    global disque
    to_write = bytearray()
    with open("HDD.bin", "wb") as file:
        for i, j in itertools.product(range(DISK_SIZE), range(128)):
            hex_string = "0" * (4 - len(hex(disque[i][j])[2:])) + hex(disque[i][j])[2:]
            hex_string = hex_string[2] + hex_string[3] + hex_string[0] + hex_string[1]
            to_write.extend(bytes.fromhex(hex_string)+bytes.fromhex("0000"))
        file.write(to_write)
    cprint(COLOR_INFO, "Dump fini !")

def p_folder_to_disk(local_place="/"):
    # TODO : fix pouvoir transferer des fichiers de edit_folder directement (ça marche que en récursif)
    if len(local_place) > 1 and local_place[0] == local_place[1] == "/":
        local_place = local_place[1:]
    if IS_LINUX: path_folder = f"{os.getcwd()}{EDIT_FOLDER}{local_place}"
    else: path_folder = f"{os.getcwd()}{EDIT_FOLDER}{local_place}".replace("/", "\\")
    for file_folder in os.listdir(path_folder):
        if os.path.isfile(f"{path_folder}/{file_folder}"):
            cprint(COLOR_INFO, f"writing file {file_folder} in {local_place}")
            make_file(local_place, file_folder)
            with open(path_folder + ("/" if IS_LINUX else "\\") + file_folder, "rb") as f:
                bit_stream = f.read()
                # mettre un byte par uint32 pour éviter les soucis, ++ faire un décalage de 1
                bit_stream = [x+1 for x in list(bit_stream)] + [0] * (3 - len(bit_stream) % 3)
            # if string_final != b"": # SI JAMAIS SOUCI REMETTRE CETTE LIGNE MAIS NORMALEMENT SA FAIT PAS DE SOUCI
            write_in_file(f"{local_place}/{file_folder}", bit_stream, len(bit_stream))
        else:
            cprint(COLOR_INFO, f"writing folder {file_folder} in {local_place}")
            make_dir(local_place, file_folder)
            p_folder_to_disk((f"/{local_place}" if local_place != "/" else "") + "/" + file_folder)

# INTERN FUNCTIONS

def i_next_free(rec = 0):
    x = 0
    for i in range(rec + 1):
        while p_read_sectors_ATA_PIO(x)[0] & 0x8000:
            x += 1
        if i != rec:
            x += 1
    return x

def i_creer_dossier(nom):
    folder_id = i_next_free()
    list_to_write = [0] * 128
    list_to_write[0] = 0xC000 # 0x8000 + 0x4000 (secteur occupé + dossier ici)

    if len(nom) > 20:
        p_print_and_exit("Erreur: le nom du dossier est trop long")

    for list_index, i in enumerate(range(len(nom)), start=1):
        list_to_write[list_index] = ord(nom[i])
    p_write_sectors_ATA_PIO(folder_id, list_to_write)

    return folder_id # on renvoie l'id du dossier
    
def i_creer_index_de_fichier(nom):
    if len(nom) > 20:
        p_print_and_exit("Erreur: le nom du dossier est trop long")

    location = i_next_free()
    location_file = i_next_free(1)

    # TODO : Vérifier qu'un fichier avec le même nom n'existe pas déja au même endroit

    # write index
    index_to_write = [0] * 128
    index_to_write[0] = 0xA000 # 0x8000 + 0x2000 (secteur occupé + index de fichier ici)
    for list_index, i in enumerate(range(len(nom)), start=1):
        index_to_write[list_index] = ord(nom[i])
    index_to_write[127] = location_file
    # cprint(COLOR_INFO, index_to_write)
    p_write_sectors_ATA_PIO(location, index_to_write)

    #write file
    file_to_write = [0] * 128
    file_to_write[0] = 0x9000 # 0x8000 + 0x1000 (secteur occupé + fichier ici)
    file_to_write[127] = 0 # suite du fichier si besoin (0 = y a pas)
    p_write_sectors_ATA_PIO(location_file, file_to_write)

    return location # on return l'id du secteur du fichier

def i_set_data_to_file(data, data_size, file_index):
    # free
    file_index = p_read_sectors_ATA_PIO(file_index)[127]
    if not p_read_sectors_ATA_PIO(file_index)[0] & 0xa000:
        p_print_and_exit("Le secteur n'est pas un fichier !")
    suite = file_index
    while suite:
        # cprint(COLOR_INFO, f"free {suite}")
        suite = i_free_file_and_get_next(suite)
    
    # write
    for i in range(data_size // 126 + 1):
        # cprint(COLOR_INFO, f"{i}, write in {file_index}")
        part = [0] * 128
        ui = 1
        part[0] = 0x9000 # (secteur occupé + fichier ici)
        for j in range(126):
            if i*126 + j >= data_size:
                ui = 0
                break
            part[j + 1] = data[i * 126 + j]
        if ui:
            part[127] = i_next_free(1)
        p_write_sectors_ATA_PIO(file_index, part)
        file_index = part[127]
    
def i_free_file_and_get_next(file_part):
    file = p_read_sectors_ATA_PIO(file_part)
    suite = file[127]
    for i in range(128):
        file[i] = 0
    return suite

def i_add_item_to_dir(file_id, folder_id):
    dossier = p_read_sectors_ATA_PIO(folder_id)
    for i in range(21, 128):
        if dossier[i] == 0:
            dossier[i] = file_id
            break
    else:
        # TODO : si le dossier n'a plus de secteurs, en ajouter un
        # i_add_item_to_dir(file_id, dossier[127])
        p_print_and_exit("i_add_item_to_dir plus de place, TODO")

def i_get_dir_content(id):
    folder = p_read_sectors_ATA_PIO(id)
    if not folder[0] & 0x8000:
        p_print_and_exit("Erreur, le secteur est vide")
    if not folder[0] & 0x4000:
        p_print_and_exit("Erreur, le secteur n'est pas un dossier")
    liste_contenu = []
    # directement dans le premier secteur
    for i in range(21, 128): # 21 = juste après le nom du dossier
        pointeur = folder[i]
        if pointeur != 0:
            liste_contenu.append(pointeur)
    # TODO : ajouter la lecture dans les secteurs suivants
    liste_noms = []
    liste_id = []
    for item in liste_contenu:
        content = p_read_sectors_ATA_PIO(item)
        content = content[1:21]
        content = "".join([chr(x) for x in content]).replace("\x00", "")
        liste_noms.append(content)
        liste_id.append(item)
    return (liste_noms, liste_id)
    

def i_path_to_id(path): # path = ["/", "dossier1", "dossier2", "fichier"]
    if path == "/":
        return 0
    path = i_parse_path(path)
    if path[0] == "/":
        path[0] = 0
    (liste_noms, liste_id) = i_get_dir_content(path[0])
    path = path[1:]
    if len(path) == 1:
        x = 0
        for element in liste_noms:
            if element == path[0]:
                break
            x += 1
        return liste_id[x]
    while len(path) != 1:
        x = 0
        for element in liste_noms:
            if element == path[0]:
                break
            x += 1
        contenu_path_0 = liste_id[x]
        (liste_noms, liste_id) = i_get_dir_content(contenu_path_0)
        path = path[1:]
    if path[0] not in liste_noms:
        return -1
    x = 0
    for element in liste_noms:
        if element == path[0]:
            break
        x += 1
    contenu_path_0 = liste_id[x]
    return contenu_path_0

def i_parse_path(path:str) -> list:
    path = path.split("/")
    path[0] = "/"
    return path

# FUNCTIONS TO USE

def get_used_sectors(disk_size):
    total = 0
    for i in range(disk_size):
        file = p_read_sectors_ATA_PIO(i)
        if file[0] & 0x8000:
            total += 1
    return total

def make_dir(path, folder_name):
    dossier = i_creer_dossier(folder_name)
    id_to_set = i_path_to_id(path)
    i_add_item_to_dir(dossier, id_to_set)
    return dossier

def make_file(path, file_name):
    fichier_test = i_creer_index_de_fichier(file_name)
    id_to_set = i_path_to_id(path)
    i_add_item_to_dir(fichier_test, id_to_set)
    return fichier_test

def write_in_file(path, data, data_size):
    id_to_set = i_path_to_id(path)
    i_set_data_to_file(data, data_size, id_to_set)

def get_file_size(path):
    id_file_index = i_path_to_id(path)
    if not p_read_sectors_ATA_PIO(id_file_index)[0] & 0xa000:
        p_print_and_exit("Le secteur n'est pas un fichier")
    sector_size = 0
    while p_read_sectors_ATA_PIO(id_file_index)[127]:
        sector_size += 1
        id_file_index = p_read_sectors_ATA_PIO(id_file_index)[127]
    return sector_size

def does_path_exists(path):
    # without a try except
    return i_path_to_id(path) != -1

# MAIN PROGRAM

i_creer_dossier("/")
p_folder_to_disk()
p_disk_to_file()
