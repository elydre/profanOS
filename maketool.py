import sys, os

# SETUP

DIRECTORY = ["kernel", "drivers", "drivers/ata", "cpu", "libc"]

CC = "gcc"
CFLAGS = "-g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -fno-pie"

OUT_DIR = "out"

COLOR_INFO = (120, 250, 161)
COLOR_EXEC = (170, 170, 170)


last_modif = lambda path: os.stat(path).st_mtime
file_exists = lambda path: os.path.exists(path) and os.path.isfile(path)
file_in_dir = lambda directory, extension: [file for file in os.listdir(directory) if file.endswith(extension)]
out_file_name = lambda file_path: f"{OUT_DIR}/{file_path.split('/')[-1].split('.')[0]}.o"
file1_newer = lambda file1, file2: last_modif(file1) > last_modif(file2) if file_exists(file1) and file_exists(file2) else False
need_rebuild = lambda file: file1_newer(file, out_file_name(file)) or not file_exists(out_file_name(file))

def cprint(color, text, end="\n"):
    r, g, b = color
    print(f"\033[38;2;{r};{g};{b}m{text}\033[0m", end=end)

def print_and_exec(command):
    global RBF
    RBF = True
    cprint(COLOR_EXEC, command)
    os.system(command)

def gen_need_dict():
    need, out = {"c":[], "h": [], "asm":[]}, []
    for dir in DIRECTORY:
        need["c"].extend([f"{dir}/{file}" for file in file_in_dir(dir, ".c")])
        out.extend([out_file_name(file) for file in file_in_dir(dir, ".c")])
        need["h"].extend([f"{dir}/{file}" for file in file_in_dir(dir, ".h")])
        need["asm"].extend([f"{dir}/{file}" for file in file_in_dir(dir, ".asm")])
        out.extend([out_file_name(file) for file in file_in_dir(dir, ".asm")])  

    for file in need["h"]:
        if file1_newer(file, "profanOS.bin"):
            cprint(COLOR_INFO, f"header '{file}' was modified, need to rebuild all")
            del need["h"]
            return need, out
    
    del need["h"]
    
    for file in [file for file in need["asm"] if file1_newer(out_file_name(file), file)]:
        need["asm"].remove(file)       
            
    for file in [file for file in need["c"] if file1_newer(out_file_name(file), file)]:
        need["c"].remove(file)
    
    return need, out

def bin_image():
    global RBF
    RBF = False
    need, out = gen_need_dict()
    if not os.path.exists(OUT_DIR):
        cprint(COLOR_INFO, f"creating '{OUT_DIR}' directory")
        os.makedirs(OUT_DIR)

    cprint(COLOR_INFO, f"{len(need['c'])} files to compile")

    if file1_newer("boot/bootsect.asm", f"{OUT_DIR}/bootsect.bin") or not file_exists(f"{OUT_DIR}/bootsect.bin"):
        print_and_exec(f"nasm boot/bootsect.asm -f bin -o {OUT_DIR}/bootsect.bin")

    if need_rebuild("boot/kernel_entry.asm"):
        print_and_exec(f"nasm boot/kernel_entry.asm -f elf -o {OUT_DIR}/kernel_entry.o")
        
    for file in need["c"]:
        print_and_exec(f"{CC} {CFLAGS} -c {file} -o {out_file_name(file)}")

    for file in need["asm"]:
        print_and_exec(f"nasm {file} -f elf -o {out_file_name(file)}")

    if RBF:
        in_files = f"{OUT_DIR}/kernel_entry.o " + " ".join(out)
        print_and_exec(f"ld -m elf_i386 -G -o {OUT_DIR}/kernel.bin -Ttext 0x1000 {in_files} --oformat binary")
        print_and_exec(f"cat {OUT_DIR}/bootsect.bin {OUT_DIR}/kernel.bin > profanOS.bin")

def make_help():
    aide = (
        ("make install", "install dependencies (debian/ubuntu)"),
        ("make", "build profanOS.bin"),
        ("make iso", "build image and convert it to iso"),
        ("make clean", "delete all files in out directory"),
        ("make fullclean", "clean + delete .bin and .iso"),
        ("make hdd", "create a empty HDD"),
        ("make run", "run the profanOS.bin in qemu"),
        ("make irun", "run the profanOS.iso in qemu"),
    )
    for command, description in aide:
        cprint(COLOR_INFO ,f"{command.upper():<15} {description}")


assos = {
    "bin_image": bin_image,
    "help": make_help
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

if __name__ == "__main__":
    main()
