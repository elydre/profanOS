from genericpath import isfile
from threading import Thread
import PIL.Image
import sys, os
# SETUP

SRC_DIRECTORY = ["boot", "kernel", "drivers", "cpu", "libc"]

INCLUDE_DIR = "include"
INCLUDE_SUB = [".", "driver", "cpu", "gui"]

ZAPPS_DIR = "zapps"

OUT_DIR = "out"

HDD_MAP = {
    "bin": f"{OUT_DIR}/zapps/*",
    "sys": None,
    "user": "sys_dir/user/*",
    "zada": "sys_dir/zada/*",
}

CC = "gcc"
CPPC = "g++"

CFLAGS = f"-g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -fno-pie -I ./{INCLUDE_DIR}"
ZAPPS_FLAGS = "-g -ffreestanding -Wall -Wno-unused -Wextra -fno-exceptions -m32"

# SETTINGS

COMPCT_CMDS = True

COLOR_INFO = (120, 250, 161)
COLOR_EXEC = (170, 170, 170)
COLOR_EROR = (255, 0, 0)


last_modif = lambda path: os.stat(path).st_mtime
file_exists = lambda path: os.path.exists(path) and os.path.isfile(path)
def zapps_file_in_dir(directory, extention):
    liste = []
    for file in os.listdir(directory):
        if os.path.isfile(f"{directory}/{file}"):
            if file.endswith(extention):
                liste.append(f"{directory}/{file}")
        else:
            liste.extend(zapps_file_in_dir(f"{directory}/{file}", extention))
    return liste
file_in_dir = lambda directory, extension: [file for file in os.listdir(directory) if file.endswith(extension)]
out_file_name = lambda file_path, sub_dir: f"{OUT_DIR}/{sub_dir}/{file_path.split('/')[-1][:-2]}.o"
file1_newer = lambda file1, file2: last_modif(file1) > last_modif(file2) if file_exists(file1) and file_exists(file2) else False


def cprint(color, text, end="\n"):
    r, g, b = color
    print(f"\033[38;2;{r};{g};{b}m{text}\033[0m", end=end)

def print_and_exec(command):
    try: shell_len = os.get_terminal_size().columns
    except Exception: shell_len = 180
    if COMPCT_CMDS and len(command) > shell_len:
        cprint(COLOR_EXEC, f"{command[:shell_len - 3]}...")
    else: cprint(COLOR_EXEC, command)
    code = os.system(command)
    if code != 0:
        cprint(COLOR_EROR, f"error {code}")
        sys.exit(code >> 8)

def gen_need_dict():
    need, out = {"c":[], "h": [], "asm":[]}, []
    for dir in SRC_DIRECTORY:
        try:
            need["h"].extend([f"{dir}/{file}" for file in file_in_dir(dir, ".h")])
            need["c"].extend([f"{dir}/{file}" for file in file_in_dir(dir, ".c")])
            need["asm"].extend([f"{dir}/{file}" for file in file_in_dir(dir, ".asm")])
            out.extend([out_file_name(file, "kernel") for file in file_in_dir(dir, ".c")])
            out.extend([out_file_name(file, "kernel") for file in file_in_dir(dir, ".asm")])
        except FileNotFoundError:
            cprint(COLOR_EROR, f"{dir} directory not found")

    for dir in INCLUDE_SUB:
        fulldir = f"./{INCLUDE_DIR}/{dir}"
        try: need["h"].extend([f"{fulldir}/{file}" for file in file_in_dir(fulldir, ".h")])
        except FileNotFoundError: cprint(COLOR_EROR, f"{fulldir} directory not found")

    for file in need["h"]:
        if file1_newer(file, "profanOS.elf"):
            cprint(COLOR_INFO, f"header '{file}' was modified, need to rebuild all")
            del need["h"]
            return need, out
    
    del need["h"]

    for file in [file for file in need["asm"] if file1_newer(out_file_name(file, "kernel"), file)]:
        need["asm"].remove(file)       

    for file in [file for file in need["c"] if file1_newer(out_file_name(file, "kernel"), file)]:
        need["c"].remove(file)
    
    return need, out

def elf_image():
    need, out = gen_need_dict()
    if not os.path.exists(f"{OUT_DIR}/kernel"):
        cprint(COLOR_INFO, f"creating '{OUT_DIR}/kernel' directory")
        os.makedirs(f"{OUT_DIR}/kernel")

    if len(need['c']): cprint(COLOR_INFO, f"{len(need['c'])} files to compile")

    def f_temp(file, type):
        global total
        if type == "c":
            print_and_exec(f"{CC} -c {file} -o {out_file_name(file, 'kernel')} {CFLAGS}")
        elif type == "asm":
            print_and_exec(f"nasm -f elf32 {file} -o {out_file_name(file, 'kernel')}")
        total -= 1

    global total
    total = len(need["c"]) + len(need["asm"])
    for file in need["c"]:
        Thread(target=f_temp, args=(file, "c")).start()
        
    for file in need["asm"]:
        Thread(target=f_temp, args=(file, "asm")).start()

    while total: pass # on a besoin d'attendre que tout soit fini
    
    if need["c"] or need["asm"]:
        in_files = " ".join(out)
        print_and_exec(f"ld -m elf_i386 -T link.ld {in_files} -o profanOS.elf")

def build_zapps():
    def build_zapp(name, fname):
        global total
        print_and_exec(f"{CC if name.endswith('.c') else CPPC} {ZAPPS_FLAGS} -c {name} -o {fname}.o -I ./zapps")
        print_and_exec(f"ld -m elf_i386 -e main -o {fname}.pe {fname}.o")
        print_and_exec(f"objcopy -O binary {fname}.pe {fname}.full -j .text -j .data -j .rodata -j .bss")
        print_and_exec(f"sed '$ s/\\x00*$//' {fname}.full > {fname}.bin")
        total -= 1

    cprint(COLOR_INFO, "building zapps...")
    zapps_list = zapps_file_in_dir("zapps", ".c") + zapps_file_in_dir("zapps", ".cpp")
    zapps_list_clean = [x for x in zapps_list if not x.startswith("zapps/Projets")]
    if not os.path.exists(f"{OUT_DIR}/zapps"):
        cprint(COLOR_INFO, f"creating '{OUT_DIR}/zapps' directory")
        os.makedirs(f"{OUT_DIR}/zapps")

    for file in zapps_list:
        if sum(x == "/" for x in file) > 1:
            dir_name = file[:max([max(x for x in range(len(file)) if file[x] == "/")])]
            if not os.path.exists(f"{OUT_DIR}/{dir_name}"):
                print(f"making {dir_name}")
                os.makedirs(f"{OUT_DIR}/{dir_name}")
                
    zapps_list = zapps_list_clean
                
    global total
    total = len(zapps_list)
    for name in zapps_list:
        fname = f"{OUT_DIR}/{''.join(name.split('.')[:-1])}"
        if file1_newer(f"{fname}.bin", f"{ZAPPS_DIR}/{name}"): 
            total -= 1
            continue
        Thread(target=build_zapp, args=(name, fname)).start()
    while total : pass # on attends que tout soit fini
    
    # on vire les .pe, .full et .o intermediaires
    for ext in ["pe", "full", "o"]:
        print_and_exec(f"rm -Rf ./out/zapps/*.{ext}")
        print_and_exec(f"rm -Rf ./out/zapps/*/*.{ext}")

def make_help():
    aide = (
        ("make",        "build profanOS kernel (elf file)"),
        ("make iso",    "build bootable iso with grub"),
        ("make disk",   "build disk image with zapps"),
        ("make clean",  "delete all files in out directory"),
        ("make fullclean", "delete all build files"),
        ("make run",    "run the profanOS.elf in qemu"),
        ("make irun",   "run the profanOS.iso in qemu"),
    )
    for command, description in aide:
        cprint(COLOR_INFO ,f"{command.upper():<15} {description}")

def make_iso(force = False):
    if file_exists("profanOS.iso") and file1_newer("profanOS.iso", "profanOS.elf") and not force:
        return cprint(COLOR_INFO, "profanOS.iso is up to date")
    cprint(COLOR_INFO, "building iso...")
    print_and_exec(f"mkdir -p {OUT_DIR}/isodir/boot/grub")
    print_and_exec(f"cp profanOS.elf {OUT_DIR}/isodir/boot/profanOS.elf")
    print_and_exec(f"cp boot/menu.lst {OUT_DIR}/isodir/boot/grub/menu.lst")
    print_and_exec(f"cp boot/stage2_eltorito {OUT_DIR}/isodir/boot/grub/stage2_eltorito")
    print_and_exec(f"mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -A profanOS -input-charset iso8859-1 -boot-info-table -o profanOS.iso {OUT_DIR}/isodir")

def gen_disk(force=False, with_src=False):  # sourcery skip: low-code-quality
    if file_exists("HDD.bin") and not force: return
    # en cas de problème de build du disk, taper 'make fullclean'
    # puis mettre en commentaire la ligne suivante          (^_^ )
    build_zapps()

    cprint(COLOR_INFO, "generating HDD.bin...")
    print_and_exec(f"rm -Rf {OUT_DIR}/disk")
    for dir in HDD_MAP:
        print_and_exec(f"mkdir -p {OUT_DIR}/disk/{dir}")
        if HDD_MAP[dir] is None: continue
        print_and_exec(f"cp -r {HDD_MAP[dir]} {OUT_DIR}/disk/{dir} || true")
    if with_src:
        print_and_exec(f"mkdir -p {OUT_DIR}/disk/src")
        for dir_name in SRC_DIRECTORY + [ZAPPS_DIR] + [INCLUDE_DIR]:
            print_and_exec(f"cp -r {dir_name} {OUT_DIR}/disk/src")
    cprint(COLOR_EXEC, "Correcly copying projects")
    try:
        for dossier in os.listdir(f"./{OUT_DIR}/disk/bin/Projets"):
            print_and_exec(f"make -C zapps/Projets/{dossier}/ run")
            print_and_exec(f"rm -Rf {OUT_DIR}/disk/bin/Projets/{dossier}/*")
            print_and_exec(f"cp -r zapps/Projets/{dossier}/*.bin  {OUT_DIR}/disk/bin/Projets/{dossier}/")
            print_and_exec(f"rm -Rf zapps/Projets/{dossier}/*.bin")
    except Exception:
        cprint(COLOR_EROR, "Error while copying projects, passing...")

    # transform every image into .img, the format of profanOS
    liste_images = []
    for extention in ["jpg", "png"]:
        liste_images.extend(zapps_file_in_dir("out", extention))
    for file in liste_images:
        file_location = file[:max([max(x for x in range(len(file)) if file[x] == "/")])]
        file_name = file.split("/")[-1].split(".")[0]

        # on transforme l'image en une liste de couleurs 6 bits
        image = PIL.Image.open(file)
        pixels = list(image.getdata())
        width, height = image.size
        liste_pixels = [pixels[i * width:(i + 1) * width] for i in range(height)]
        liste_couleurs = []
        for ligne in liste_pixels:
            for pixel in ligne:
                r, g, b = pixel
                color = (g//64 << 4) + (r//64 << 2) + b//64
                liste_couleurs.append(color)
        liste_couleurs = [[("0" if x < 10 else "") + str(x) for x in liste_couleurs[i * width:(i + 1) * width]] for i in range(height)]
        # on écrit le fichier
        with open(f"{file_location}/{file_name}.img", "w") as f:
            f.write(f"{len(liste_couleurs)}|{len(liste_couleurs[0])}|")
            for ligne in liste_couleurs:
                for couleur in ligne:
                    f.write(f"{couleur}|")

        # on vire l'ancienne image
        os.remove(file)

    print_and_exec("python3 makefsys.py")

def qemu_run(iso_run = False):
    elf_image()
    if iso_run: make_iso()
    gen_disk(False)
    cprint(COLOR_INFO, "starting qemu...")
    if iso_run: print_and_exec("qemu-system-i386 -cdrom profanOS.iso -drive file=HDD.bin,format=raw -boot order=d")
    else: print_and_exec("qemu-system-i386 -kernel profanOS.elf -drive file=HDD.bin,format=raw -boot order=a")

assos = {
    "elf_image": elf_image,
    "help": make_help,
    "disk": lambda: gen_disk(False),
    "diskf": lambda: gen_disk(True),
    "disk_src": lambda: gen_disk(True, True),
    "iso": lambda: make_iso(True),
    "run": lambda: qemu_run(False),
    "irun": lambda: qemu_run(True),
}

def main():
    if len(sys.argv) < 2:
        print("please use the Makefile")
        return
    arg = sys.argv[1]

    if arg in assos:
        assos[arg]()
    else:
        print("unknown argument, please use the Makefile")
        exit(1)

if __name__ == "__main__": main()
else: print("mhhh, akyzo ?")
