#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
#   === maketool.py : 2024 ===                                                #
#                                                                             #
#    Python script to build profanOS images                        .pi0iq.    #
#                                                                 d"  . `'b   #
#    This file is part of profanOS and is released under          q. /|\  "   #
#    the terms of the GNU General Public License                   `// \\     #
#                                                                  //   \\    #
#   === elydre : https://github.com/elydre/profanOS ===         #######  \\   #
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

import datetime
import os
import sys
import threading
from time import sleep

# SETTINGS
SHOW_CMD    = False     # show full command line     [0]
COMPCT_LINE = True      # cut line if too long       [1]
LOG_FILE    = True      # write build logs file      [1]
DEBUG_MKFS  = False     # run makefsys with valgrind [0]

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
}

CC   = "gcc"
LD   = "ld"
SHRD = "gcc -shared"

KERNEL_SRC = [f"kernel/{e}" for e in os.listdir("kernel")] + ["boot"]
KERNEL_HEADERS = ["include/kernel"]

ZHEADERS = ["include/zlibs", "include/addons"]

ZAPPS_DIR = "zapps"
ZLIBS_DIR = "zlibs"
ZAPPS_BIN = "sys" # zapps/sys
ZLIBS_MOD = "mod" # zlibs/mod

LINK_FILENAME = "_link.txt" # multi file link
INCLUDE_DIR   = "include"   # multi file include

LINK_LINE_MAX = 12          # max line for link instructions

CFLAGS     = "-m32 -march=i686 -ffreestanding -fno-exceptions -fno-stack-protector -nostdinc -nostdlib "
CFLAGS    += "-Wall -Wextra -D__profanOS__"

CC_OPTIM   = "-O3 -fno-omit-frame-pointer"

KERN_FLAGS = f"{CFLAGS} -fno-pie -I include/kernel"
ZAPP_FLAGS = f"{CFLAGS} -Werror"
ZLIB_FLAGS = f"{CFLAGS} -Wno-unused -Werror -fPIC"

KERN_LINK  = f"-m elf_i386 -T {TOOLS_DIR}/link_kernel.ld -Map {OUT_DIR}/make/kernel.map"
LD_FLAGS   = "-m elf_i386 -nostdlib"

QEMU_SPL   = "qemu-system-i386"
QEMU_KVM   = "qemu-system-i386 -enable-kvm"

QEMU_FLAGS = "-serial stdio"
QEMU_AUDIO = "-audiodev pa,id=snd0 -machine pcspk-audiodev=snd0"

COLOR_INFO = (120, 250, 161)
COLOR_EXEC = (170, 170, 170)
COLOR_EROR = (255, 100, 80)

def cprint(color, text, end="\n"):
    r, g, b = color
    print(f"\033[38;2;{r};{g};{b}m{text}\033[0m", end=end)

if os.getenv("PROFANOS_OPTIM") == "1":
    cprint(COLOR_INFO, "profanOS optimisation enabled")
    ZAPP_FLAGS += f" {CC_OPTIM}"

if os.getenv("PROFANOS_KIND") == "1":
    cprint(COLOR_INFO, "profanOS kind mode enabled")
    ZAPP_FLAGS = ZAPP_FLAGS.replace("-Wall", "").replace("-Wextra", "").replace("-Werror", "")

for e in ZHEADERS:
    if os.path.exists(e):
        ZAPP_FLAGS += f" -I {e}"
        ZLIB_FLAGS += f" -I {e}"

last_modif    = lambda path: os.stat(path).st_mtime
file_exists   = lambda path: os.path.exists(path) and os.path.isfile(path)
files_in_dir  = lambda directory, extension: [file for file in os.listdir(directory) if file.endswith(extension)]
out_file_name = lambda file_path, sub_dir: f"{OUT_DIR}/{sub_dir}/{file_path.split('/')[-1].split('.')[0]}.o"
remove_ext    = lambda name: ''.join(name.split('.')[:-1])
file1_newer   = lambda file1, file2: last_modif(file1) > last_modif(file2) if (
                                   file_exists(file1) and file_exists(file2)) else False

def files_in_dir_rec(directory, extention):
    liste = []

    for file in os.listdir(directory):
        if not os.path.isfile(f"{directory}/{file}"):
            liste.extend(files_in_dir_rec(f"{directory}/{file}", extention))

        elif file.endswith(extention):
            liste.append(f"{directory}/{file}")

    return liste

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
            need["c"].extend([f"{dir}/{file}" for file in files_in_dir(dir, ".c")])
            need["asm"].extend([f"{dir}/{file}" for file in files_in_dir(dir, ".asm")])
            out.extend([out_file_name(file, "kernel") for file in files_in_dir(dir, ".c")])
            out.extend([out_file_name(file, "kernel") for file in files_in_dir(dir, ".asm")])
        except FileNotFoundError:
            cprint(COLOR_EROR, f"{dir} directory not found")

    for dir in KERNEL_HEADERS:
        for fulldir in [dir] + [f"{dir}/{subdir}" for subdir in os.listdir(dir) if os.path.isdir(f"{dir}/{subdir}")]:
            try: need["h"].extend([f"{fulldir}/{file}" for file in files_in_dir(fulldir, ".h")])
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


def build_disk_elfs():
    if not os.path.exists(f"{OUT_DIR}/zapps"):
        cprint(COLOR_EXEC, f"[mkdir] {OUT_DIR}/zapps")
        os.makedirs(f"{OUT_DIR}/zapps")

    if not os.path.exists(f"{OUT_DIR}/zlibs"):
        cprint(COLOR_EXEC, f"[mkdir] {OUT_DIR}/zlibs")
        os.makedirs(f"{OUT_DIR}/zlibs")

    if not os.path.exists(f"{OUT_DIR}/make"):
        cprint(COLOR_EXEC, f"[mkdir] {OUT_DIR}/make")
        os.makedirs(f"{OUT_DIR}/make")

    if not file_exists(f"{OUT_DIR}/make/entry_bin.o") or file1_newer(
                "{TOOLS_DIR}/entry_bin.c", f"{OUT_DIR}/make/entry_bin.o"):
        cprint(COLOR_EXEC, f"{TOOLS_DIR}/entry_bin.c")
        print_and_exec(f"{CC} -c {TOOLS_DIR}/entry_bin.c -o {OUT_DIR}/make/entry_bin.o {ZAPP_FLAGS}")

    if not file_exists(f"{OUT_DIR}/make/entry_elf.o") or file1_newer(
                "{TOOLS_DIR}/entry_elf.c", f"{OUT_DIR}/make/entry_elf.o"):
        cprint(COLOR_EXEC, f"{TOOLS_DIR}/entry_elf.c")
        print_and_exec(f"{CC} -c {TOOLS_DIR}/entry_elf.c -o {OUT_DIR}/make/entry_elf.o {ZAPP_FLAGS}")

    def build_c_to_sys(name, fname):
        global total
        print_info_line(name)
        print_and_exec(f"{CC} -c {name} -o {fname}.o {ZAPP_FLAGS}")
        print_and_exec(f"{LD} {LD_FLAGS} -T {TOOLS_DIR}/link_bin.ld -o " +
                       f"{fname}.elf {OUT_DIR}/make/entry_bin.o {fname}.o")
        print_and_exec(f"rm {fname}.o")
        total -= 1

    def build_c_to_mod(name, fname):
        global total
        print_info_line(name)
        print_and_exec(f"{CC} -c {name} -o {fname}.o {ZLIB_FLAGS}")
        print_and_exec(f"{SHRD} -m32 -nostdlib -o {fname}.pok {fname}.o")
        print_and_exec(f"rm {fname}.o")
        total -= 1

    def build_c_to_elf(name, fname, liblist):
        # build object file and link it using shared libs
        print_info_line(name)
        required_libs = []
        with open(name, "r") as f:
            for _ in range(LINK_LINE_MAX):
                first_line = f.readline()
                if not first_line.startswith("// @LINK:"):
                    continue
                required_libs = first_line.split(":")[1].replace("\n", "").replace(",", " ").split()
                break

        for lib in required_libs:
            if lib not in liblist:
                cprint(COLOR_EROR, f"maketool: {name}: library '{lib}' not found\n{' '*10}available: {liblist}")
                os._exit(1)

        print_and_exec(f"{CC} -c {name} -o {fname}.o {ZAPP_FLAGS}")
        print_and_exec(f"{LD} {LD_FLAGS} -T {TOOLS_DIR}/link_elf.ld -L {OUT_DIR}/zlibs -o " +
                       f"{fname}.elf {OUT_DIR}/make/entry_elf.o {fname}.o -lc " +
                       ' '.join([f'-l{lib[3:]}' for lib in required_libs]))
        print_and_exec(f"rm {fname}.o")

        global total
        total -= 1

    def build_c_to_obj(name, fname, is_lib):
        global total
        print_info_line(name)
        if is_lib:
            flags = ZLIB_FLAGS
        else:
            flags = ZAPP_FLAGS + f" -I {'/'.join(name.split('/')[0:3])}/{INCLUDE_DIR}"
        print_and_exec(f"{CC} {flags} -c {name} -o {fname}.o")
        total -= 1

    def link_multifile_elf(name, libs_name):
        objs = files_in_dir_rec(f"{OUT_DIR}/{name}", ".o")
        print_info_line(f"[link] {name}.elf")
        # read link file to get required libs
        required_libs = []
        try:
            with open(f"{name}/{LINK_FILENAME}", "r") as f:
                lines = f.readlines()
                for line in lines:
                    required_libs.extend(line.replace("\n", "").replace(",", " ").split())
        except FileNotFoundError: pass
        for lib in required_libs:
            if lib not in libs_name:
                cprint(COLOR_EROR, f"maketool: {name}: library '{lib}' not found\n{' '*10}available: {libs_name}")
                os._exit(1)
        print_and_exec(f"{LD} {LD_FLAGS} -T {TOOLS_DIR}/link_elf.ld -L {OUT_DIR}/zlibs " +
                          f"-o {OUT_DIR}/{name}.elf {OUT_DIR}/make/entry_elf.o {' '.join(objs)} -lc " +
                            ' '.join([f'-l{lib[3:]}' for lib in required_libs]))

    # detect zlibs
    lib_build_list = []
    mod_build_list = []

    for dir_name in os.listdir(ZLIBS_DIR):
        if not os.path.isdir(f"{ZLIBS_DIR}/{dir_name}"):
            cprint(COLOR_EROR, f"file '{ZLIBS_DIR}/{dir_name}' is not a directory")
            exit(1)

        if dir_name == ZLIBS_MOD:
            mod_build_list.extend(files_in_dir_rec(f"{ZLIBS_DIR}/{dir_name}", ".c"))
            continue

        lib_build_list.extend(files_in_dir_rec(f"{ZLIBS_DIR}/{dir_name}", ".c"))

    # detect zapps
    bin_build_list = []
    dir_build_list = []
    elf_build_list = []

    for dir_name in os.listdir(ZAPPS_DIR):
        if not os.path.isdir(f"{ZAPPS_DIR}/{dir_name}"):
            cprint(COLOR_EROR, f"file '{ZAPPS_DIR}/{dir_name}' is not a directory")
            exit(1)

        if dir_name == ZAPPS_BIN:
            for file in os.listdir(f"{ZAPPS_DIR}/{dir_name}"):
                if os.path.isdir(f"{ZAPPS_DIR}/sys/{file}"):
                    cprint(COLOR_EROR, f"direcory {file} not supported in '{ZAPPS_DIR}/sys'")
                    exit(1)
                bin_build_list.append(f"{ZAPPS_DIR}/{dir_name}/{file}")
            continue

        for file in os.listdir(f"{ZAPPS_DIR}/{dir_name}"):
            if os.path.isdir(f"{ZAPPS_DIR}/{dir_name}/{file}"):
                if file in [file.split("/")[-2] for file in dir_build_list]:
                    cprint(COLOR_EROR, f"{file}: multiple command with same name")
                    exit(1)
                dir_build_list.extend(files_in_dir_rec(f"{ZAPPS_DIR}/{dir_name}/{file}", ".c"))
            elif file.endswith(".c"):
                elf_build_list.append(f"{ZAPPS_DIR}/{dir_name}/{file}")
            else:
                cprint(COLOR_EROR, f"unknown file type '{file}' in '{ZAPPS_DIR}/{dir_name}'")
                exit(1)

    # check double file names (cmd/*/name.c and cmd/*/name/file1.c)
    monofile = [remove_ext(file.split("/")[-1]) for file in elf_build_list] + list(
               set([file.split("/")[-2] for file in dir_build_list]))
    monofile = set([e for e in monofile if monofile.count(e) > 1])

    if monofile:
        cprint(COLOR_EROR, f"multiple command with same name: {', '.join(monofile)}")
        exit(1)

    # check if zapps need to be rebuild
    total_bin = len(bin_build_list)
    total_dir = len(dir_build_list)
    total_elf = len(elf_build_list)
    total_lib = len(lib_build_list)
    total_mod = len(mod_build_list)

    elf_build_list = [file for file in elf_build_list if not file1_newer(
            f"{OUT_DIR}/{file.replace('.c', '.elf')}", file)]
    bin_build_list = [file for file in bin_build_list if not file1_newer(
            f"{OUT_DIR}/{file.replace('.c', '.bin')}", file)]
    mod_build_list = [file for file in mod_build_list if not file1_newer(
            f"{OUT_DIR}/{file.replace('.c', '.pok')}", file)]

    lib_build_list = [file for file in lib_build_list if not (file1_newer(
            f"{OUT_DIR}/{file.replace('.c', '.o')}", file) and file1_newer(
            f"{OUT_DIR}/zlibs/{file.split('/')[1]}.so", file))]

    dir_build_list = [file for file in dir_build_list if not (file1_newer(
            f"{OUT_DIR}/{file.replace('.c', '.o')}", file) and file1_newer(
            f"{OUT_DIR}/{'/'.join(file.split('/')[0:3])}.elf", file))]

    cprint(COLOR_INFO, f"| lib files : {len(lib_build_list)}/{total_lib}")
    cprint(COLOR_INFO, f"| kernel mod: {len(mod_build_list)}/{total_mod}")
    cprint(COLOR_INFO, f"| system bin: {len(bin_build_list)}/{total_bin}")
    cprint(COLOR_INFO, f"| single elf: {len(elf_build_list)}/{total_elf}")
    cprint(COLOR_INFO, f"| multi file : {len(dir_build_list)}/{total_dir}")

    # create directories
    for file in elf_build_list + bin_build_list + lib_build_list + mod_build_list + dir_build_list:
        dir_name = file[:max([max(x for x in range(len(file)) if file[x] == "/")])]

        if not os.path.exists(f"{OUT_DIR}/{dir_name}"):
            cprint(COLOR_EXEC, f"[mkdir] {OUT_DIR}/{dir_name}")
            os.makedirs(f"{OUT_DIR}/{dir_name}")

    global total
    total = len(lib_build_list)

    for name in lib_build_list:
        fname = f"{OUT_DIR}/{remove_ext(name)}"
        threading.Thread(target = build_c_to_obj, args=(name, fname, 1)).start()

    while total:
        # wait for all threads to finish
        sleep(0.05)

    # linking libs
    for name in list(set([f"{name.split('/')[1]}" for name in lib_build_list])):
        objs = files_in_dir_rec(f"{OUT_DIR}/zlibs/{name}", ".o")
        print_info_line(f"[link] zlibs/{name}.so")
        print_and_exec(f"{SHRD} -m32 -nostdlib -o {OUT_DIR}/zlibs/{name}.so {' '.join(objs)}")

    total = len(elf_build_list) + len(bin_build_list) + len(mod_build_list) + len(dir_build_list)

    # get .so files
    libs_name = [e[:-3] for e in files_in_dir(f"{OUT_DIR}/zlibs", ".so")]

    for name in mod_build_list:
        fname = f"{OUT_DIR}/{remove_ext(name)}"
        threading.Thread(target = build_c_to_mod, args=(name, fname)).start()

    for name in elf_build_list:
        fname = f"{OUT_DIR}/{remove_ext(name)}"
        threading.Thread(target = build_c_to_elf, args=(name, fname, libs_name)).start()

    for name in bin_build_list:
        fname = f"{OUT_DIR}/{remove_ext(name)}"
        threading.Thread(target = build_c_to_sys, args=(name, fname)).start()

    for name in dir_build_list:
        fname = f"{OUT_DIR}/{remove_ext(name)}"
        threading.Thread(target = build_c_to_obj, args=(name, fname, 0)).start()

    while total:
        # wait for all threads to finish
        sleep(0.05)

    # linking multi file zapps
    for name in list(set(["/".join(name.split("/")[0:3]) for name in dir_build_list])):
        link_multifile_elf(name, libs_name)

def make_iso(force = False, more_option = False):
    elf_image()
    gen_disk()

    if file_exists("profanOS.iso") and file1_newer("profanOS.iso", "kernel.elf") and (
            file1_newer("profanOS.iso", "initrd.bin") and not force):
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


def get_kernel_version(print_info = True, add_editing = False):
    path = os.path.dirname(os.path.abspath(__file__))

    info = "unknown"

    with open(f"{path}/../include/kernel/system.h", "r") as f:
        for line in f:
            if "KERNEL_VERSION" in line:
                info = line.split(" ")[-1][1:-2]
                if not add_editing:
                    break
            if "KERNEL_EDIT" not in line:
                continue
            info += f" ({line.split(' ')[-1][1:-2]})"
            break

    if print_info:
        print(info)
    return info


def write_build_logs():
    print_info_line("generating build logs")

    try:
        user_name = os.getlogin()
    except Exception:
        user_name = "unknown"

    linux_dist = os.popen("lsb_release -d").read().splitlines()[0].split(":")[1].strip()

    text = "UTC TIME  " + datetime.datetime.now(datetime.timezone.utc).strftime(
        "%Y-%m-%d %H:%M:%S"
    ) + "\n"
    text += f"    HOST  {os.uname().nodename} ({linux_dist}, {os.uname().sysname}) by {user_name}\n"
    text += f"  KERNEL  profanOS {get_kernel_version(False, True)}\n"
    text += f"      CC  {os.popen(f'{CC} --version').read().splitlines()[0]}\n"
    text += f"      LD  {os.popen('ld --version').read().splitlines()[0]}\n"
    text += f"  PYTHON  {os.popen('python3 --version').read().splitlines()[0]}\n"
    text += f"    GRUB  {os.popen('grub-mkrescue --version').read().splitlines()[0]}\n"
    text += f"GIT HASH  {os.popen('git rev-parse HEAD').read().splitlines()[0]}\n"
    text += f"GIT REPO  {os.popen('git remote get-url origin').read().splitlines()[0]}"
    text += f" [{os.popen('git rev-parse --abbrev-ref HEAD').read().splitlines()[0]}]\n"

    with open(f"{OUT_DIR}/disk/user/build.log", "w") as f:
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

    build_disk_elfs()

    cprint(COLOR_INFO, "generating initrd.bin...")
    print_and_exec(f"rm -Rf {OUT_DIR}/disk")

    for dir_name in HDD_MAP:
        print_and_exec(f"mkdir -p {OUT_DIR}/disk/{dir_name}")

        if HDD_MAP[dir_name] is None: continue

        if isinstance(HDD_MAP[dir_name], str):
            if dir_name == "lib":
                print_and_exec(f"cp -r {HDD_MAP[dir_name]}/*.* " + ' '.join([f'{HDD_MAP[dir_name]}/{file}'
                        for file in os.listdir(HDD_MAP[dir_name]) if not file.startswith('lib')
                    ]) + f" {OUT_DIR}/disk/{dir_name}")

            elif dir_name == "bin":
                for file in os.listdir(HDD_MAP[dir_name]):
                    if not os.path.isdir(f"{HDD_MAP[dir_name]}/{file}"):
                        print_and_exec(f"cp {HDD_MAP[dir_name]}/{file} {OUT_DIR}/disk/{dir_name}")
                    if not os.path.exists(f"{OUT_DIR}/disk/{dir_name}/{file}"):
                        os.makedirs(f"{OUT_DIR}/disk/{dir_name}/{file}")
                    print_and_exec(f"cp -r {HDD_MAP[dir_name]}/{file}/*.* {OUT_DIR}/disk/{dir_name}/{file}")

            else:
                print_and_exec(f"cp -r {HDD_MAP[dir_name]}/* {OUT_DIR}/disk/{dir_name} 2> /dev/null || true")

            continue

        for e in HDD_MAP[dir_name]:
            if not os.path.exists(e): continue
            print_and_exec(f"cp -r {e}/* {OUT_DIR}/disk/{dir_name}")

    if with_src:
        add_src_to_disk()

    if LOG_FILE:
        write_build_logs()

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
    print_and_exec(("valgrind "if DEBUG_MKFS else "") + f"./{OUT_DIR}/make/makefsys \"$(pwd)/{OUT_DIR}/disk\"")


def qemu_run(iso_run = True, kvm = False, audio = False):
    if iso_run: make_iso()

    gen_disk(False)
    qemu_cmd = QEMU_KVM if kvm else QEMU_SPL

    qemu_args = QEMU_FLAGS
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
        ("make addons",     "start graphical addons selection"),
        ("make gaddons",    "install automatically recommended addons"),
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
    cprint(COLOR_INFO, " MAKE ADDONS BDISK ISO to build the iso with all options")
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
