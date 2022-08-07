import sys, os

# SETUP

DIRECTORY = ["kernel", "drivers", "drivers/ata", "cpu", "libc"]

CC = "gcc"
CFLAGS = "-g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -fno-pie"

OUT_DIR = "out"

# recuperer les arguments
if len(sys.argv) < 2:
    print("please use the Makefile")
    exit(1)
else:
    arg = sys.argv[1]


last_modif = lambda path: os.stat(path).st_mtime
file_exists = lambda path: os.path.exists(path) and os.path.isfile(path)
file_in_dir = lambda directory, extension: [file for file in os.listdir(directory) if file.endswith(extension)]
out_file_name = lambda file_path: f"{OUT_DIR}/{file_path.split('/')[-1].split('.')[0]}.o"
file1_newer = lambda file1, file2: last_modif(file1) > last_modif(file2) if file_exists(file1) and file_exists(file2) else False
need_rebuild = lambda file: file1_newer(file, out_file_name(file)) or not file_exists(out_file_name(file))

def green_print(string):
    print(f"\033[96m{string}\033[0m")

def print_and_exec(command):
    global RBF
    RBF = True
    print(command)
    os.system(command)

def gen_need_dict():
    need = {"c":[], "h": [], "asm":[]}
    for dir in DIRECTORY:
        need["c"].extend([f"{dir}/{file}" for file in file_in_dir(dir, ".c")])
        need["h"].extend([f"{dir}/{file}" for file in file_in_dir(dir, ".h")])
        need["asm"].extend([f"{dir}/{file}" for file in file_in_dir(dir, ".asm")])    

    for file in need["h"]:
        if file1_newer(file, "profanOS.bin"):
            green_print(f"header '{file}' was modified, need to rebuild all")
            del need["h"]
            return need
    
    del need["h"]
    
    for file in [file for file in need["asm"] if file1_newer(out_file_name(file), file)]:
        need["asm"].remove(file)       
            
    for file in [file for file in need["c"] if file1_newer(out_file_name(file), file)]:
        need["c"].remove(file)
    
    return need

def bin_image():
    global RBF
    RBF, need = False, gen_need_dict()
    if not os.path.exists(OUT_DIR):
        os.makedirs(OUT_DIR)

    green_print(f"{len(need['c'])} c files to compile")

    if file1_newer("boot/bootsect.asm", f"{OUT_DIR}/bootsect.bin") or not file_exists(f"{OUT_DIR}/bootsect.bin"):
        print_and_exec(f"nasm boot/bootsect.asm -f bin -o {OUT_DIR}/bootsect.bin")

    if need_rebuild("boot/kernel_entry.asm"):
        print_and_exec(f"nasm boot/kernel_entry.asm -f elf -o {OUT_DIR}/kernel_entry.o")
        
    for file in need["c"]:
        print_and_exec(f"{CC} {CFLAGS} -c {file} -o {out_file_name(file)}")

    for file in need["asm"]:
        print_and_exec(f"nasm {file} -f elf -o {out_file_name(file)}")

    if RBF:
        print_and_exec(f"ld -m elf_i386 -G -o {OUT_DIR}/kernel.bin -Ttext 0x1000 {' '.join(f'{OUT_DIR}/{f}' for f in file_in_dir(OUT_DIR, '.o'))} --oformat binary")
        print_and_exec(f"cat {OUT_DIR}/bootsect.bin {OUT_DIR}/kernel.bin > profanOS.bin")

def mk_hdd():
    print_and_exec("dd if=/dev/zero of=HDD.bin bs=1024 count=1024")

bin_image()
mk_hdd()
