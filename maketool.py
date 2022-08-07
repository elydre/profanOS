import sys, os

# SETUP

CC = "gcc"
CFLAGS = "-g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -fno-pie"

OUT_DIR = "out"

# recuperer les arguments
args = sys.argv[1:]
pre = args[0]

edit_output = lambda path: f"{OUT_DIR}/{path.split('/')[-1]}"
edit_all_output = lambda paths: " ".join(map(edit_output, paths))

last_modif = lambda path: os.stat(path).st_mtime
file_exists = lambda path: os.path.exists(path) and os.path.isfile(path)

def chek_date():
    res = False
    for i in range(2, 2 + len(args[2:])):
        args[i] = edit_output(args[i])
        if pre == "ld":
            if not (file_exists(args[1]) and last_modif(args[i]) < last_modif(args[1])):
                res = True

        elif not (file_exists(args[2]) and last_modif(args[2]) > last_modif(args[1])):
            res = True
    return res


if not chek_date():
    print("aucun fichier a compiler")

else:
    if not os.path.exists(OUT_DIR):
        os.mkdir(OUT_DIR)

    if pre == "gcc":
        cmd = f"{CC} {CFLAGS} -c {args[1]} -o {args[2]}"
        print(cmd)
        os.system(cmd)

    elif pre == "ld":
        cmd = f"ld -m elf_i386 -G -o {args[1]} -Ttext 0x1000 {' '.join(args[2:])} --oformat binary"
        print(cmd)
        os.system(cmd)

    elif pre == "nasm-bin":
        cmd = f"nasm -f bin {args[1]} -o {args[2]}"
        print(cmd)
        os.system(cmd)

    elif pre == "nasm-elf":
        cmd = f"nasm -f elf {args[1]} -o {args[2]}"
        print(cmd)
        os.system(cmd)
