import sys, os

# SETTINGS

OUT_DIR = "out/"

COMPCT_CMDS = True

COLOR_INFO = (120, 250, 161)
COLOR_EXEC = (170, 170, 170)
COLOR_EROR = (255, 0, 0)

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
        sys.exit(code)


def build_prog(name):
    print(f"Building {name}...")
    print_and_exec(f"gcc -g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -c {name}.c -o {OUT_DIR}{name}.o")
    print_and_exec(f"ld -m elf_i386 -e start -o {OUT_DIR}{name}.pe {OUT_DIR}{name}.o")
    print_and_exec(f"objcopy -O binary {OUT_DIR}{name}.pe {OUT_DIR}{name}.full -j .text -j .data -j .rodata -j .bss")
    print_and_exec(f"sed '$ s/\\x00*$//' {OUT_DIR}{name}.full > {OUT_DIR}{name}.bin")

def build_all():
    print_and_exec(f"mkdir -p {OUT_DIR}")
    files = [f[:-2] for f in os.listdir() if f.endswith(".c")]
    for f in files: build_prog(f)


build_all()
