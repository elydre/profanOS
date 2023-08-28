import datetime
import os
import sys
import threading

# SETUP

SRC_DIRECTORY = [f"kernel/{e}" for e in os.listdir("kernel")] + ["boot"]
INCLUDE_DIR = ["include/kernel", "include/zlibs"]

ZAPPS_DIR = "zapps"
ZLIBS_DIR = "zlibs"
TOOLS_DIR = "tools"
OUT_DIR   = "out"

HDD_MAP = {
    "bin": f"{OUT_DIR}/zapps",
    "lib": f"{OUT_DIR}/zlibs",
    "sys": None, # "sys_dir/sys",
    "user": "sys_dir/user",
    "zada": [
        "sys_dir/zada",
        f"{OUT_DIR}/zada",
    ],
    "tmp" : None
}

CC   = "gcc"
CPPC = "g++"

CFLAGS     = "-m32 -ffreestanding -Wall -Wextra -fno-exceptions -fno-stack-protector -march=i686"
KERN_FLAGS = f"{CFLAGS} -fno-pie -I include/kernel"
ZAPP_FLAGS = f"{CFLAGS} -Wno-unused -Werror -I include/zlibs"

KERN_LINK = f"-m elf_i386 -T {TOOLS_DIR}/klink.ld -Map {OUT_DIR}/make/kernel.map"

QEMU_SPL = "qemu-system-i386"
QEMU_KVM = "kvm"

QEMU_SERIAL = "-serial stdio"
QEMU_AUDIO  = "-audiodev pa,id=snd0 -machine pcspk-audiodev=snd0"

# SETTINGS

COMPCT_CMDS = True
HBL_FILE    = True

COLOR_INFO = (120, 250, 161)
COLOR_EXEC = (170, 170, 170)
COLOR_EROR = (255, 100, 80)


last_modif = lambda path: os.stat(path).st_mtime
file_exists = lambda path: os.path.exists(path) and os.path.isfile(path)
file_in_dir = lambda directory, extension: [file for file in os.listdir(directory) if file.endswith(extension)]
out_file_name = lambda file_path, sub_dir: f"{OUT_DIR}/{sub_dir}/{file_path.split('/')[-1].split('.')[0]}.o"
file1_newer = lambda file1, file2: last_modif(file1) > last_modif(file2) if file_exists(file1) and file_exists(file2) else False


def find_app_lib(directory, extention):
    liste = []

    for file in os.listdir(directory):
        if not os.path.isfile(f"{directory}/{file}"):
            liste.extend(find_app_lib(f"{directory}/{file}", extention))

        elif file.endswith(extention):
            liste.append(f"{directory}/{file}")

    return liste


def cprint(color, text, end="\n"):
    r, g, b = color
    print(f"\033[38;2;{r};{g};{b}m{text}\033[0m", end=end)


def print_and_exec(command):
    try:
        shell_len = os.get_terminal_size().columns
    except Exception:
        shell_len = 180

    if COMPCT_CMDS and len(command) > shell_len:
        cprint(COLOR_EXEC, f"{command[:shell_len - 3]}...")
    else: cprint(COLOR_EXEC, command)

    code = os.system(command) >> 8

    if code != 0:
        cprint(COLOR_EROR, f"command '{command}' failed with code {code}")
        os._exit(code)


def gen_need_dict():
    need, out = {"c":[], "h": [], "asm":[]}, []

    for dir in SRC_DIRECTORY:
        try:
            need["c"].extend([f"{dir}/{file}" for file in file_in_dir(dir, ".c")])
            need["asm"].extend([f"{dir}/{file}" for file in file_in_dir(dir, ".asm")])
            out.extend([out_file_name(file, "kernel") for file in file_in_dir(dir, ".c")])
            out.extend([out_file_name(file, "kernel") for file in file_in_dir(dir, ".asm")])
        except FileNotFoundError:
            cprint(COLOR_EROR, f"{dir} directory not found")

    for dir in INCLUDE_DIR:
        for fulldir in [dir] + [f"{dir}/{subdir}" for subdir in os.listdir(dir) if os.path.isdir(f"{dir}/{subdir}")]:
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
            print_and_exec(f"{CC} -c {file} -o {out_file_name(file, 'kernel')} {KERN_FLAGS}")

        elif type == "asm":
            print_and_exec(f"nasm -f elf32 {file} -o {out_file_name(file, 'kernel')}")

        total -= 1

    global total
    total = len(need["c"]) + len(need["asm"])

    for file in need["c"]:
        threading.Thread(target=f_temp, args=(file, "c")).start()

    for file in need["asm"]:
        threading.Thread(target=f_temp, args=(file, "asm")).start()

    while total: pass # on a besoin d'attendre que tout soit fini

    if not os.path.exists(f"{OUT_DIR}/make"):
        cprint(COLOR_INFO, f"creating '{OUT_DIR}/make' directory")
        os.makedirs(f"{OUT_DIR}/make")

    if need["c"] or need["asm"]:
        in_files = " ".join(out)
        print_and_exec(f"ld {KERN_LINK} {in_files} -o profanOS.elf")


def build_app_lib():
    if not file_exists(f"{OUT_DIR}/make/zentry.o") or file1_newer("{TOOLS_DIR}/zentry.c", f"{OUT_DIR}/make/zentry.o"):
        cprint(COLOR_INFO, "building zentry...")
        print_and_exec(f"mkdir -p {OUT_DIR}/make")
        print_and_exec(f"gcc -c {TOOLS_DIR}/zentry.c -o {OUT_DIR}/make/zentry.o {ZAPP_FLAGS}")

    def build_file(name, fname):
        global total
        print_and_exec(f"{CC if name.endswith('.c') else CPPC} -c {name} -o {fname}.o {ZAPP_FLAGS}")
        print_and_exec(f"ld -m elf_i386 -T {TOOLS_DIR}/zlink.ld -o {fname}.pe {OUT_DIR}/make/zentry.o {fname}.o")
        print_and_exec(f"objcopy -O binary {fname}.pe {fname}.bin -j .text -j .data -j .rodata -j .bss")
        print_and_exec(f"rm {fname}.o {fname}.pe")
        total -= 1

    cprint(COLOR_INFO, "building zapps and zlibs")

    build_list = find_app_lib(ZAPPS_DIR, ".c")
    build_list += find_app_lib(ZAPPS_DIR, ".cpp")

    build_list += find_app_lib(ZLIBS_DIR, ".c")
    build_list += find_app_lib(ZLIBS_DIR, ".cpp")

    if not os.path.exists(f"{OUT_DIR}/zapps"):
        cprint(COLOR_INFO, f"creating '{OUT_DIR}/zapps' directory")
        os.makedirs(f"{OUT_DIR}/zapps")

    if not os.path.exists(f"{OUT_DIR}/zlibs"):
        cprint(COLOR_INFO, f"creating '{OUT_DIR}/zlibs' directory")
        os.makedirs(f"{OUT_DIR}/zlibs")

    skip_count = 0

    for file in build_list:
        if sum(x == "/" for x in file) <= 1:
            continue

        if file.split("/")[-2].startswith("_"):
            skip_count += 1
            build_list = [x for x in build_list if x != file]
            continue

        dir_name = file[:max([max(x for x in range(len(file)) if file[x] == "/")])]

        if not os.path.exists(f"{OUT_DIR}/{dir_name}"):
            cprint(COLOR_EXEC, f"creating '{OUT_DIR}/{dir_name}' directory")
            os.makedirs(f"{OUT_DIR}/{dir_name}")

    build_list = [x for x in build_list if not x.startswith("zapps/projets")]

    # check if zapps need to be rebuild
    updated_list = [file for file in build_list if not file1_newer(f"{OUT_DIR}/{file.replace('.c', '.bin').replace('.cpp', '.bin')}", file)]
    cprint(COLOR_INFO, f"{len(updated_list)} zapps and zlibs to build (active: {len(build_list)}, skipped: {skip_count}, total: {len(build_list) + skip_count})")
    build_list = updated_list

    if not build_list: return

    global total
    total = len(build_list)

    for name in build_list:
        fname = f"{OUT_DIR}/{''.join(name.split('.')[:-1])}"

        if file1_newer(f"{fname}.bin", f"{ZAPPS_DIR}/{name}"):
            total -= 1
            continue

        threading.Thread(target = build_file, args = (name, fname)).start()

    while total : pass # on attends que tout soit fini


def make_iso(force = False, more_option = False):
    elf_image()

    gen_disk()

    if file_exists("profanOS.iso") and file1_newer("profanOS.iso", "profanOS.elf") and not force:
        return cprint(COLOR_INFO, "profanOS.iso is up to date")

    cprint(COLOR_INFO, "building iso...")
    print_and_exec(f"mkdir -p {OUT_DIR}/isodir/boot/grub")
    print_and_exec(f"cp profanOS.elf {OUT_DIR}/isodir/boot/")

    print_and_exec(f"echo TITE | cat initrd.bin - > {OUT_DIR}/isodir/boot/initrd.bin")

    if more_option:
        print_and_exec(f"cp boot/full.cfg {OUT_DIR}/isodir/boot/grub/grub.cfg")
    else:
        print_and_exec(f"cp boot/classic.cfg {OUT_DIR}/isodir/boot/grub/grub.cfg")

    print_and_exec("grub-mkrescue -o profanOS.iso out/isodir/")


def get_kernel_version(print_info = True):
    path = os.path.dirname(os.path.abspath(__file__))

    with open(f"{path}/../include/kernel/system.h", "r") as f:
        for line in f:
            if "KERNEL_VERSION" not in line: continue
            info = line.split(" ")[-1][1:-2]
            if print_info: print(info)
            return info


def write_build_logs():
    cprint(COLOR_EXEC, "writing build logs...")

    text = "- initrd.bin BUILD LOGS -\n"
    text += "UTC build time: " + datetime.datetime.now(datetime.timezone.utc).strftime(
        "%Y-%m-%d %H:%M:%S"
    ) + "\n"
    text += f"machine name:   {os.uname().nodename} ({os.uname().sysname})\n"
    text += f"build for:      profanOS {get_kernel_version(False)}\n"
    text += f"gcc version:    {os.popen(f'{CC} --version').read().splitlines()[0]}\n"
    text += f"ld version:     {os.popen('ld --version').read().splitlines()[0]}\n"
    text += f"python version: {os.popen('python3 --version').read().splitlines()[0]}\n"

    with open(f"{OUT_DIR}/disk/user/hbl.txt", "w") as f:
        f.write(text)


def add_src_to_disk():
    print_and_exec(f"mkdir -p {OUT_DIR}/disk/src")
    for dir_name in [ZAPPS_DIR] + [ZLIBS_DIR]:
        if not os.path.exists(dir_name): continue
        print_and_exec(f"cp -r {dir_name} {OUT_DIR}/disk/src")

    print_and_exec(f"mkdir -p {OUT_DIR}/disk/src/include")
    for dir_name in INCLUDE_DIR:
        if not os.path.exists(dir_name): continue
        print_and_exec(f"cp -r {dir_name} {OUT_DIR}/disk/src/include")

    print_and_exec(f"mkdir -p {OUT_DIR}/disk/src/kernel")
    for dir_name in SRC_DIRECTORY:
        if not os.path.exists(dir_name): continue
        print_and_exec(f"cp -r {dir_name} {OUT_DIR}/disk/src/kernel")


def gen_disk(force=False, with_src=False):

    if file_exists("initrd.bin") and not force: return

    build_app_lib()

    cprint(COLOR_INFO, "generating initrd.bin...")
    print_and_exec(f"rm -Rf {OUT_DIR}/disk")

    for dir in HDD_MAP:
        print_and_exec(f"mkdir -p {OUT_DIR}/disk/{dir}")

        if HDD_MAP[dir] is None: continue

        if isinstance(HDD_MAP[dir], str):
            print_and_exec(f"cp -r {HDD_MAP[dir]}/* {OUT_DIR}/disk/{dir} || true")
            continue

        for dir_name in HDD_MAP[dir]:
            if not os.path.exists(dir_name): continue
            print_and_exec(f"cp -r {dir_name}/* {OUT_DIR}/disk/{dir}")

    if with_src:
        add_src_to_disk()

    try:
        for dossier in os.listdir(f"{OUT_DIR}/disk/bin/projets"):
            print_and_exec(f"make -C zapps/projets/{dossier}/ build")
            print_and_exec(f"cp zapps/projets/{dossier}/*.bin {OUT_DIR}/disk/bin/projets/")
            print_and_exec(f"rm -Rf {OUT_DIR}/disk/bin/projets/{dossier}/ zapps/projets/{dossier}/*.bin")
    except Exception as e:
        cprint(COLOR_EROR, f"Error while building projets {e}")

    if HBL_FILE: write_build_logs()

    print_and_exec(f"cp {TOOLS_DIR}/zentry.c {OUT_DIR}/disk/sys/")
    print_and_exec(f"cp {TOOLS_DIR}/tcclib.c {OUT_DIR}/disk/sys/")
    print_and_exec(f"cp {TOOLS_DIR}/zlink.ld {OUT_DIR}/disk/sys/")
    print_and_exec(f"cp -r include/zlibs {OUT_DIR}/disk/sys/include/")
    print_and_exec(f"cp {OUT_DIR}/make/kernel.map {OUT_DIR}/disk/sys/ || true")

    if not file_exists(f"{OUT_DIR}/make/makefsys.bin"):
        cprint(COLOR_INFO, "building makefsys...")
        print_and_exec(f"mkdir -p {OUT_DIR}/make")
        print_and_exec(f"gcc -o {OUT_DIR}/make/makefsys.bin -Wall -Wextra {TOOLS_DIR}/makefsys/*/*.c")

    cprint(COLOR_INFO, "building initrd.bin...")
    print_and_exec(f"./{OUT_DIR}/make/makefsys.bin \"$(pwd)/{OUT_DIR}/disk\"")


def qemu_run(iso_run = True, kvm = False, audio = False):
    if iso_run: make_iso()

    gen_disk(False)
    qemu_cmd = QEMU_KVM if kvm else QEMU_SPL

    qemu_args = QEMU_SERIAL
    if audio: qemu_args += f" {QEMU_AUDIO}"

    cprint(COLOR_INFO, "starting qemu...")

    if iso_run: print_and_exec(f"{qemu_cmd} -cdrom profanOS.iso -drive file=initrd.bin,format=raw -boot order=d {qemu_args}")
    else: print_and_exec(f"{qemu_cmd} -kernel profanOS.elf -drive file=initrd.bin,format=raw -boot order=a {qemu_args}")

def extract_disk():
    if not file_exists("initrd.bin"):
        cprint(COLOR_EROR, "initrd.bin not found")
        return

    if not file_exists(f"{OUT_DIR}/make/makefsys.bin"):
        cprint(COLOR_INFO, "building makefsys...")
        print_and_exec(f"mkdir -p {OUT_DIR}/make")
        print_and_exec(f"gcc -o {OUT_DIR}/make/makefsys.bin -Wall -Wextra {TOOLS_DIR}/makefsys/*/*.c")

    cprint(COLOR_INFO, "extracting initrd.bin...")
    print_and_exec(f"./{OUT_DIR}/make/makefsys.bin 42")


def make_help():
    aide = (
        ("make [info]", "show this help message"),

        ("make elf",        "build the kernel in elf format"),
        ("make iso",        "build the iso image of profanOS"),
        ("make miso",       "build the iso with more grub options"),

        ("make disk",       "build classic disk image"),
        ("make srcdisk",    "build disk image with source code"),
        ("make xtrdisk",    "extract the disk image"),

        ("make addons",     "download all addons in disk source"),

        ("make clean",      "delete all build files"),
        ("make fclean",     "reset the repository"),

        ("make run",        "run the profanOS.iso in qemu"),
        ("make erun",       "run the profanOS.elf in qemu"),
        ("make krun",       "run the profanOS.iso with kvm"),
        ("make srun",       "run the profanOS.iso with sound"),
    )

    for command, description in aide:
        cprint(COLOR_INFO ,f"{command.upper():<15} {description}")

    cprint(COLOR_INFO, "\nYou can cross the command like:")
    cprint(COLOR_INFO, " MAKE DISK RUN to force the disk generation and run it")
    cprint(COLOR_INFO, " MAKE ADDONS SRCDISK MISO to build the disk with all options")
    cprint(COLOR_INFO, "You can also use tools/ directory to more options...")


assos = {
    "elf": elf_image,
    "help": make_help,
    "disk": lambda: gen_disk(True),
    "srcdisk": lambda: gen_disk(True, True),
    "xtrdisk": lambda: extract_disk(),
    "iso": lambda: make_iso(True),
    "miso": lambda: make_iso(True, True),
    "run": lambda: qemu_run(True),
    "erun": lambda: qemu_run(False),
    "krun": lambda: qemu_run(True, True),
    "srun": lambda: qemu_run(True, False, True),
    "kver": get_kernel_version,
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
