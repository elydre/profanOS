import datetime
import os
import sys
import threading

# SETTINGS
SHOW_CMD    = False
COMPCT_LINE = True
HBL_FILE    = True

# SETUP
TOOLS_DIR = "tools"
OUT_DIR   = "out"

HDD_MAP = {
    "bin": f"{OUT_DIR}/zapps",
    "lib": f"{OUT_DIR}/zlibs",
    "sys": f"{OUT_DIR}/sys",
    "user": "sys_dir/user",
    "zada": [
        "sys_dir/zada",
        f"{OUT_DIR}/zada",
    ],
    "tmp" : None
}

CC   = "gcc"
CPPC = "g++"
LD   = "ld"
SHRD = "gcc -shared"

KERNEL_SRC = [f"kernel/{e}" for e in os.listdir("kernel")] + ["boot"]
KERNEL_HEADERS = ["include/kernel"]

ZAPPS_DIR = "zapps"
ZLIBS_DIR = "zlibs"
ZHEADERS = ["include/zlibs", "include/addons"]
ZAPPS_BIN = "zapps/sys"
ZLIBS_MOD = "zlibs/mod"

CFLAGS     = "-m32 -ffreestanding -Wall -Wextra -fno-exceptions -fno-stack-protector -march=i686 -nostdinc"
KERN_FLAGS = f"{CFLAGS} -fno-pie -I include/kernel"
ZAPP_FLAGS = f"{CFLAGS} -Wno-unused -Werror"

KERN_LINK = f"-m elf_i386 -T {TOOLS_DIR}/link_kernel.ld -Map {OUT_DIR}/make/kernel.map"

QEMU_SPL = "qemu-system-i386"
QEMU_KVM = "qemu-system-i386 -enable-kvm"

QEMU_SERIAL = "-serial stdio"
QEMU_AUDIO  = "-audiodev pa,id=snd0 -machine pcspk-audiodev=snd0"

COLOR_INFO = (120, 250, 161)
COLOR_EXEC = (170, 170, 170)
COLOR_EROR = (255, 100, 80)

for e in ZHEADERS:
    if os.path.exists(e):
        ZAPP_FLAGS += f" -I {e}"

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

    if SHOW_CMD:
        if COMPCT_LINE and len(command) > shell_len:
            cprint(COLOR_EXEC, f"{command[:shell_len - 3]}...")
        else: cprint(COLOR_EXEC, command)

    code = os.system(command) >> 8

    if code != 0:
        cprint(COLOR_EROR, f"command '{command}' failed with code {code}")
        os._exit(code)

def print_info_line(text):
    if SHOW_CMD:
        return

    try:
        shell_len = os.get_terminal_size().columns
    except Exception:
        shell_len = 180

    if COMPCT_LINE and len(text) > shell_len:
        cprint(COLOR_EXEC, f"{text[:shell_len - 3]}...")
    else: cprint(COLOR_EXEC, text)

def gen_need_dict():
    need, out = {"c":[], "h": [], "asm":[]}, []

    for dir in KERNEL_SRC:
        try:
            need["c"].extend([f"{dir}/{file}" for file in file_in_dir(dir, ".c")])
            need["asm"].extend([f"{dir}/{file}" for file in file_in_dir(dir, ".asm")])
            out.extend([out_file_name(file, "kernel") for file in file_in_dir(dir, ".c")])
            out.extend([out_file_name(file, "kernel") for file in file_in_dir(dir, ".asm")])
        except FileNotFoundError:
            cprint(COLOR_EROR, f"{dir} directory not found")

    for dir in KERNEL_HEADERS:
        for fulldir in [dir] + [f"{dir}/{subdir}" for subdir in os.listdir(dir) if os.path.isdir(f"{dir}/{subdir}")]:
            try: need["h"].extend([f"{fulldir}/{file}" for file in file_in_dir(fulldir, ".h")])
            except FileNotFoundError: cprint(COLOR_EROR, f"{fulldir} directory not found")

    for file in need["h"]:
        if file1_newer(file, "kernel.elf"):
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

        print_info_line(file)

        if type == "c":
            print_and_exec(f"{CC} -c {file} -o {out_file_name(file, 'kernel')} {KERN_FLAGS}")
        elif type == "asm":
            print_and_exec(f"nasm -f elf32 {file} -o {out_file_name(file, 'kernel')}")
        else:
            cprint(COLOR_EROR, f"unknown file type '{type}'")
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
        print_info_line("linking kernel.elf")
        print_and_exec(f"{LD} {KERN_LINK} {in_files} -o kernel.elf")


def build_app_lib():
    if not os.path.exists(f"{OUT_DIR}/zapps"):
        cprint(COLOR_EXEC, f"creating '{OUT_DIR}/zapps' directory")
        os.makedirs(f"{OUT_DIR}/zapps")

    if not os.path.exists(f"{OUT_DIR}/zlibs"):
        cprint(COLOR_EXEC, f"creating '{OUT_DIR}/zlibs' directory")
        os.makedirs(f"{OUT_DIR}/zlibs")

    if not os.path.exists(f"{OUT_DIR}/make"):
        cprint(COLOR_EXEC, f"creating '{OUT_DIR}/make' directory")
        os.makedirs(f"{OUT_DIR}/make")

    if not file_exists(f"{OUT_DIR}/make/entry_bin.o") or file1_newer("{TOOLS_DIR}/entry_bin.c", f"{OUT_DIR}/make/entry_bin.o"):
        cprint(COLOR_INFO, "building binary entry...")
        print_and_exec(f"{CC} -c {TOOLS_DIR}/entry_bin.c -o {OUT_DIR}/make/entry_bin.o {ZAPP_FLAGS}")

    if not file_exists(f"{OUT_DIR}/make/entry_elf.o") or file1_newer("{TOOLS_DIR}/entry_elf.c", f"{OUT_DIR}/make/entry_elf.o"):
        cprint(COLOR_INFO, "building ELF entry...")
        print_and_exec(f"{CC} -c {TOOLS_DIR}/entry_elf.c -o {OUT_DIR}/make/entry_elf.o {ZAPP_FLAGS}")

    def build_bin_file(name, fname):
        global total
        print_info_line(name)
        print_and_exec(f"{CC if name.endswith('.c') else CPPC} -c {name} -o {fname}.o {ZAPP_FLAGS}")
        print_and_exec(f"{LD} -m elf_i386 -T {TOOLS_DIR}/link_bin.ld -o {fname}.pe {OUT_DIR}/make/entry_bin.o {fname}.o")
        print_and_exec(f"objcopy -O binary {fname}.pe {fname}.bin -j .text -j .data -j .rodata -j .bss")
        print_and_exec(f"rm {fname}.o {fname}.pe")
        total -= 1

    def build_elf_file(name, fname, liblist):
        # build object file and link it using shared libs
        print_info_line(name)
        required_libs = []
        with open(name, "r") as f:
            first_line = f.readline()
            if first_line.startswith("// @LINK SHARED:"):
                required_libs = first_line.split(":")[1].replace("\n", "").replace(",", " ").split()

        for lib in required_libs:
            if lib not in liblist:
                cprint(COLOR_EROR, f"library '{lib}' is not found, available: {liblist}")
                os._exit(1)

        print_and_exec(f"{CC if name.endswith('.c') else CPPC} -c {name} -o {fname}.o {ZAPP_FLAGS}")
        print_and_exec(f"{LD} -nostdlib -m elf_i386 -T {TOOLS_DIR}/link_elf.ld -L {OUT_DIR}/zlibs -o {fname}.elf {OUT_DIR}/make/entry_elf.o {fname}.o -lc {' '.join([f'-l{lib[3:]}' for lib in required_libs])}")
        print_and_exec(f"rm {fname}.o")

        global total
        total -= 1

    def build_obj_file(name, fname):
        global total
        print_info_line(name)
        print_and_exec(f"{CC if name.endswith('.c') else CPPC} -fPIC -c {name} -o {fname}.o {ZAPP_FLAGS}")
        total -= 1

    lib_build_list = find_app_lib(ZLIBS_DIR, ".c")
    lib_build_list += find_app_lib(ZLIBS_DIR, ".cpp")
    bin_build_list = []

    for e in lib_build_list:
        if e.startswith(f"{ZLIBS_MOD}/"):
            lib_build_list = [x for x in lib_build_list if x != e]
            bin_build_list.append(e)

    elf_build_list = find_app_lib(ZAPPS_DIR, ".c")
    elf_build_list += find_app_lib(ZAPPS_DIR, ".cpp")

    for e in elf_build_list:
        if e.startswith(f"{ZAPPS_BIN}/"):
            elf_build_list = [x for x in elf_build_list if x != e]
            bin_build_list.append(e)

    for file in elf_build_list + bin_build_list + lib_build_list:
        if sum(x == "/" for x in file) <= 1:
            cprint(COLOR_EROR, f"file '{file}' is not in a subdirectory")
            exit(1)

        dir_name = file[:max([max(x for x in range(len(file)) if file[x] == "/")])]

        if not os.path.exists(f"{OUT_DIR}/{dir_name}"):
            cprint(COLOR_EXEC, f"creating '{OUT_DIR}/{dir_name}' directory")
            os.makedirs(f"{OUT_DIR}/{dir_name}")

    # check if zapps need to be rebuild
    total_elf = len(elf_build_list)
    total_bin = len(bin_build_list)
    total_lib = len(lib_build_list)

    elf_build_list = [file for file in elf_build_list if not file1_newer(f"{OUT_DIR}/{file.replace('.c', '.elf').replace('.cpp', '.elf')}", file)]
    bin_build_list = [file for file in bin_build_list if not file1_newer(f"{OUT_DIR}/{file.replace('.c', '.bin').replace('.cpp', '.bin')}", file)]
    lib_build_list = [file for file in lib_build_list if not (file1_newer(f"{OUT_DIR}/{file.replace('.c', '.o').replace('.cpp', '.o')}", file) and file1_newer(f"{OUT_DIR}/zlibs/{file.split('/')[1]}.so", file))]

    cprint(COLOR_INFO, f"{len(elf_build_list)}/{total_elf} elf, {len(bin_build_list)}/{total_bin} bin and {len(lib_build_list)}/{total_lib} lib files to compile")

    global total
    total = len(lib_build_list)

    for name in lib_build_list:
        fname = f"{OUT_DIR}/{''.join(name.split('.')[:-1])}"
        threading.Thread(target = build_obj_file, args=(name, fname)).start()

    while total: pass # on attends que tout soit fini

    # linking libs
    for name in list(dict.fromkeys([f"{name.split('/')[1]}" for name in lib_build_list])):
        objs = find_app_lib(f"{OUT_DIR}/zlibs/{name}", ".o")
        print_info_line(f"linking {name}")
        print_and_exec(f"{SHRD} -m32 -nostdlib -o {OUT_DIR}/zlibs/{name}.so {' '.join(objs)}")

    total = len(elf_build_list) + len(bin_build_list)

    # get .so files
    libs_name = [e[:-3] for e in file_in_dir(f"{OUT_DIR}/zlibs", ".so")]

    for name in elf_build_list:
        fname = f"{OUT_DIR}/{''.join(name.split('.')[:-1])}"
        threading.Thread(target = build_elf_file, args=(name, fname, libs_name)).start()

    for name in bin_build_list:
        fname = f"{OUT_DIR}/{''.join(name.split('.')[:-1])}"
        threading.Thread(target = build_bin_file, args=(name, fname)).start()

    while total: pass # on attends que tout soit fini

def make_iso(force = False, more_option = False):
    elf_image()

    gen_disk()

    if file_exists("profanOS.iso") and file1_newer("profanOS.iso", "kernel.elf") and file1_newer("profanOS.iso", "initrd.bin") and not force:
        return cprint(COLOR_INFO, "profanOS.iso is up to date")

    cprint(COLOR_INFO, "building iso...")
    print_and_exec(f"mkdir -p {OUT_DIR}/isodir/boot/grub")
    print_and_exec(f"cp kernel.elf {OUT_DIR}/isodir/boot/")
    print_and_exec(f"cp initrd.bin {OUT_DIR}/isodir/boot/")

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
    print_info_line("generating build logs")

    try:
        user_name = os.getlogin()
    except Exception:
        user_name = "unknown"

    text = "- initrd.bin BUILD LOGS -\n"
    text += "UTC build time: " + datetime.datetime.now(datetime.timezone.utc).strftime(
        "%Y-%m-%d %H:%M:%S"
    ) + "\n"
    text += f"machine name:   {os.uname().nodename} ({os.uname().sysname}) by {user_name}\n"
    text += f"build for:      profanOS kernel {get_kernel_version(False)}\n"
    text += f"CC version:     {os.popen(f'{CC} --version').read().splitlines()[0]}\n"
    text += f"ld version:     {os.popen('ld --version').read().splitlines()[0]}\n"
    text += f"python version: {os.popen('python3 --version').read().splitlines()[0]}\n"
    text += f"grub version:   {os.popen('grub-mkrescue --version').read().splitlines()[0]}\n"

    with open(f"{OUT_DIR}/disk/user/hbl.txt", "w") as f:
        f.write(text)


def add_src_to_disk():
    print_and_exec(f"mkdir -p {OUT_DIR}/disk/src")
    for dir_name in [ZAPPS_DIR] + [ZLIBS_DIR]:
        if not os.path.exists(dir_name): continue
        print_and_exec(f"cp -r {dir_name} {OUT_DIR}/disk/src")

    print_and_exec(f"mkdir -p {OUT_DIR}/disk/src/include")
    for dir_name in ZHEADERS + KERNEL_HEADERS:
        if not os.path.exists(dir_name): continue
        print_and_exec(f"cp -r {dir_name} {OUT_DIR}/disk/src/include")

    print_and_exec(f"mkdir -p {OUT_DIR}/disk/src/kernel")
    for dir_name in KERNEL_SRC:
        if not os.path.exists(dir_name): continue
        print_and_exec(f"cp -r {dir_name} {OUT_DIR}/disk/src/kernel")


def gen_disk(force=False, with_src=False):

    if file_exists("initrd.bin") and not force: return

    build_app_lib()

    cprint(COLOR_INFO, "generating initrd.bin...")
    print_and_exec(f"rm -Rf {OUT_DIR}/disk")

    for dir_name in HDD_MAP:
        print_and_exec(f"mkdir -p {OUT_DIR}/disk/{dir_name}")

        if HDD_MAP[dir_name] is None: continue

        if isinstance(HDD_MAP[dir_name], str):
            if dir_name == "lib":
                print_and_exec(f"cp -r {HDD_MAP[dir_name]}/*.* {HDD_MAP[dir_name]}/mod {OUT_DIR}/disk/{dir_name}")
                continue
            print_and_exec(f"cp -r {HDD_MAP[dir_name]}/* {OUT_DIR}/disk/{dir_name} 2> /dev/null || true")
            continue

        for e in HDD_MAP[dir_name]:
            if not os.path.exists(e): continue
            print_and_exec(f"cp -r {e}/* {OUT_DIR}/disk/{dir_name}")

    if with_src:
        add_src_to_disk()

    if HBL_FILE: write_build_logs()

    print_info_line("copy sys/ directory")
    print_and_exec(f"cp {TOOLS_DIR}/entry_elf.c {OUT_DIR}/disk/sys/zentry.c")
    print_and_exec(f"gcc -c {OUT_DIR}/disk/sys/zentry.c -o {OUT_DIR}/disk/sys/zentry.o {ZAPP_FLAGS}")
    print_and_exec(f"cp {TOOLS_DIR}/link_elf.ld {OUT_DIR}/disk/sys/")
    print_and_exec(f"cp {OUT_DIR}/make/kernel.map {OUT_DIR}/disk/sys/ 2> /dev/null || true")

    print_and_exec(f"mkdir -p {OUT_DIR}/disk/sys/include")
    for dir_name in ZHEADERS:
        if not os.path.exists(dir_name):
            continue
        for subdir in os.listdir(dir_name):
            print_and_exec(f"cp -r {dir_name}/{subdir} {OUT_DIR}/disk/sys/include")

    if not file_exists(f"{OUT_DIR}/make/makefsys"):
        cprint(COLOR_INFO, "building makefsys...")
        print_and_exec(f"mkdir -p {OUT_DIR}/make")
        print_and_exec(f"{CC} -o {OUT_DIR}/make/makefsys -Wall -Wextra -g {TOOLS_DIR}/makefsys/*/*.c")

    cprint(COLOR_INFO, "building initrd.bin...")
    print_and_exec(f"./{OUT_DIR}/make/makefsys \"$(pwd)/{OUT_DIR}/disk\"")


def qemu_run(iso_run = True, kvm = False, audio = False):
    if iso_run: make_iso()

    gen_disk(False)
    qemu_cmd = QEMU_KVM if kvm else QEMU_SPL

    qemu_args = QEMU_SERIAL
    if audio: qemu_args += f" {QEMU_AUDIO}"

    cprint(COLOR_INFO, "starting qemu...")

    if iso_run: print_and_exec(f"{qemu_cmd} -cdrom profanOS.iso -boot order=d {qemu_args}")
    else: print_and_exec(f"{qemu_cmd} -kernel kernel.elf -boot order=a {qemu_args}")

def make_help():
    help_lines = (
        ("make [help]", "show this help message"),
        None,
        ("make elf",        "build the kernel in elf format"),
        ("make iso",        "build the iso image of profanOS"),
        ("make miso",       "build the iso with more grub options"),
        None,
        ("make disk",       "build classic disk image"),
        ("make bdisk",    "build disk image with source code"),
        None,
        ("make addons",     "download all addons in disk source"),
        ("make gaddons",    "start graphical addons selection"),
        None,
        ("make clean",      "delete all build files"),
        ("make fclean",     "reset the repository"),
        None,
        ("make run",        "run the profanOS.iso in qemu"),
        ("make erun",       "run the kernel.elf in qemu"),
        ("make krun",       "run the profanOS.iso with kvm"),
        ("make srun",       "run the profanOS.iso with sound"),
    )

    for e in help_lines:
        if e is None:
            print()
            continue
        command, description = e
        cprint(COLOR_INFO ,f"{command.upper():<15} {description}")

    cprint(COLOR_INFO, "\nYou can cross the command like:")
    cprint(COLOR_INFO, " MAKE DISK RUN to force the disk generation and run it")
    cprint(COLOR_INFO, " MAKE ADDONS BDISK MISO to build the disk with all options")
    cprint(COLOR_INFO, "You can also use tools/ directory to more options...")


assos = {
    "elf": elf_image,
    "help": make_help,
    "disk": lambda: gen_disk(True),
    "bdisk": lambda: gen_disk(True, True),
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
