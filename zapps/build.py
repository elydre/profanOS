import sys, os

# SETTINGS

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
    print_and_exec(f"gcc -g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -c {name}.c -o {name}.o")
    print_and_exec(f"ld -m elf_i386 -e start -o {name}.pe {name}.o -Ttext 0x1000")
    print_and_exec(f"objcopy -O binary {name}.pe {name}.bin")
    print_and_exec(f"sed '$ s/\\x00*$//' {name}.bin > {name}.bin.tmp")
    print_and_exec(f"mv {name}.bin.tmp {name}.bin")

build_prog("togame")
print_and_exec("rm *.o *.pe")
