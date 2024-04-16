import urllib.request as urlreq
import curses, os, sys

path = os.path.dirname(os.path.abspath(__file__))
profan_path = path.rsplit(os.sep, 1)[0]

#######################################################
#######################################################

RECOMMENDED = ["tcc", "lua", "zlib", "doom"]

ADDONS = {
    "compilation tools": [
        {
            "name": "tcc",
            "description": "Small and fast C compiler",
            "files": [
                {
                    "name": "tcc",
                    "url": "https://github.com/elydre/tinycc-profan/releases/download/latest/tcc.elf",
                    "path": [profan_path, "out", "zapps", "fatpath", "tcc.elf"]
                },
                {
                    "name": "libtcc.a",
                    "url": "https://github.com/elydre/tinycc-profan/releases/download/latest/libtcc.a",
                    "path": [profan_path, "out", "zlibs", "libtcc.a"]
                },
            ]
        },
        {
            "name": "vlink",
            "description": "multi-target linker [deprecated]",
            "files": [
                {
                    "name": "vlink",
                    "url": "https://github.com/elydre/vlink-profan/releases/download/latest/vlink.elf",
                    "path": [profan_path, "out", "zapps", "fatpath", "vlink.elf"]
                },
            ]
        },
    ],
    "libraries": [
        {
            "name": "zlib",
            "description": "Compression library + gzip command",
            "files": [
                {
                    "name": "libz.so",
                    "url": "https://github.com/elydre/libs-profan/releases/download/latest/libz.so",
                    "path": [profan_path, "out", "zlibs", "libz.so"]
                },
                {
                    "name": "zlib.h",
                    "url": "https://raw.githubusercontent.com/elydre/libs-profan/main/_headers/zlib.h",
                    "path": [profan_path, "include", "addons", "zlib.h"]
                },
                {
                    "name": "gzip",
                    "url": "https://github.com/elydre/libs-profan/releases/download/latest/gzip.elf",
                    "path": [profan_path, "out", "zapps", "cmd", "gzip.elf"]
                }
            ]
        },
        {
            "name": "libupng",
            "description": "Lightweight PNG Decoding Library",
            "files": [
                {
                    "name": "libupng.so",
                    "url": "https://github.com/elydre/libs-profan/releases/download/latest/libupng.so",
                    "path": [profan_path, "out", "zlibs", "libupng.so"]
                },
                {
                    "name": "upng.h",
                    "url": "https://raw.githubusercontent.com/elydre/libs-profan/main/_headers/upng.h",
                    "path": [profan_path, "include", "addons", "upng.h"]
                },
            ]
        },
        {
            "name": "libm",
            "description": "Math library",
            "files": [
                {
                    "name": "libm.so",
                    "url": "https://github.com/elydre/libs-profan/releases/download/latest/libm.so",
                    "path": [profan_path, "out", "zlibs", "libm.so"]
                },
                {
                    "name": "math.h",
                    "url": "https://raw.githubusercontent.com/elydre/libs-profan/main/_headers/math.h",
                    "path": [profan_path, "include", "addons", "math.h"]
                },
                {
                    "name": "complex.h",
                    "url": "https://raw.githubusercontent.com/elydre/libs-profan/main/_headers/complex.h",
                    "path": [profan_path, "include", "addons", "complex.h"]
                },
                {
                    "name": "fenv.h",
                    "url": "https://raw.githubusercontent.com/elydre/libs-profan/main/_headers/fenv.h",
                    "path": [profan_path, "include", "addons", "fenv.h"]
                },
            ]
        }
    ],
    "extra shells": [
        {
            "name": "dash",
            "description": "POSIX compliant shell [experimental]",
            "files": [
                {
                    "name": "dash",
                    "url": "https://github.com/elydre/dash-profan/releases/download/latest/dash.elf",
                    "path": [profan_path, "out", "zapps", "fatpath", "dash.elf"]
                },
            ]
        },
    ],
    "graphics": [
        {
            "name": "doom",
            "description": "Raycasting first person shooter",
            "files": [
                {
                    "name": "doom",
                    "url": "https://github.com/elydre/doom-profan/releases/download/latest/doom.elf",
                    "path": [profan_path, "out", "zapps", "fatpath", "doom.elf"]
                },
                {
                    "name": "doom1.wad",
                    "url": "https://distro.ibiblio.org/slitaz/sources/packages/d/doom1.wad",
                    "path": [profan_path, "out", "zada", "doom", "DOOM1.WAD"]
                }
            ]
        },

        {
            "name": "halfix",
            "description": "x86 emulator with provided linux image",
            "files": [
                {
                    "name": "halfix",
                    "url": "https://github.com/elydre/halfix-profan/releases/download/latest/halfix.elf",
                    "path": [profan_path, "out", "zapps", "fatpath", "halfix.elf"]
                },
                {
                    "name": "bios.bin",
                    "url": "https://github.com/elydre/halfix-profan/raw/master/bios.bin",
                    "path": [profan_path, "out", "zada", "halfix", "bios.bin"]
                },
                {
                    "name": "vgabios.bin",
                    "url": "https://github.com/elydre/halfix-profan/raw/master/vgabios.bin",
                    "path": [profan_path, "out", "zada", "halfix", "vgabios.bin"]
                },
                {
                    "name": "default.cfg",
                    "url": "https://raw.githubusercontent.com/elydre/halfix-profan/master/default.conf",
                    "path": [profan_path, "out", "zada", "halfix", "default.cfg"]
                },
                {
                    "name": "linux.iso",
                    "url": "https://github.com/copy/images/raw/master/linux.iso",
                    "path": [profan_path, "out", "zada", "halfix", "linux.iso"]
                }
            ]
        },
    ],
    "interpreters": [
        {
            "name": "lua",
            "description": "Lightweight scripting language",
            "files": [
                {
                    "name": "lua",
                    "url": "https://github.com/elydre/lua-profan/releases/download/latest/lua.elf",
                    "path": [profan_path, "out", "zapps", "fatpath", "lua.elf"]
                },
            ]
        },
        {
            "name": "sulfur",
            "description": "Bytecode high-performance language",
            "files": [
                {
                    "name": "sulfur",
                    "url": "https://github.com/elydre/sulfur_lang/releases/download/latest/sulfur-profanOS-i386.elf",
                    "path": [profan_path, "out", "zapps", "fatpath", "sulfur.elf"]
                },
            ]
        },
    ],
}

ALL_ADOONS = [e for category in ADDONS for e in ADDONS[category]]

def get_addons(name: str) -> dict:
    for category in ADDONS:
        for addon in ADDONS[category]:
            if addon["name"] == name:
                return addon
    return None

#######################################################
#######################################################

def domain(url: str) -> str:
    return url.split("/")[2]

def download(url: str, path: str) -> bool:
    def show_progress(block_num, block_size, total_size):
        percent = int(block_num * block_size * 100 / total_size)
        print(f"\r | Downloaded {percent}%", end="\r")

    # get file
    try:
        urlreq.urlretrieve(url, path, show_progress)
    except Exception:
        print("\r | ERROR: Could not download file")
        return False

    return True

def download_addon(dic: dict) -> bool:
    print(f"\rInstall {dic['name']}: {dic['description']}")
    for sub in dic["files"]:
        print(f"\r Getting {dic['name']} part: {sub['name']}")
        # check if parent directory exists
        parent = os.sep.join(sub["path"][:-1])
        if not os.path.exists(parent):
            print("\r | Creating directory", parent)
            os.makedirs(parent)

        print(f"\r | Downloading {sub['url']}")

        # download
        if not download(sub["url"], os.sep.join(sub["path"])): return False

    return True

#######################################################
#######################################################

def graphic_menu(stdscr):
    curses.curs_set(0)
    stdscr.clear()
    stdscr.refresh()

    checked = [False] * len(ALL_ADOONS)

    print(len(ALL_ADOONS))

    current = 1

    def draw_menu(stdscr, checked, current):
        stdscr.clear()
        stdscr.addstr(0, 0, "Select addons to install with ENTER", curses.A_BOLD)
        stdscr.addstr(1, 0, "Q: cancel, RIGHT: info, V: validate")

        stdscr.addstr(3, 1, "Download Selected" if any(checked) else "Exit without downloading", curses.A_REVERSE if current == 0 else 0)
        stdscr.addstr(4, 1, "Unselect all" if any(checked) else "Select all", curses.A_REVERSE if current == 1 else 0)

        index = 0
        line_offset = 6
        for category in ADDONS:
            stdscr.addstr(index + line_offset, 1, category.upper(), curses.A_BOLD)
            line_offset += 1
            for addon in ADDONS[category]:
                stdscr.addstr(index + line_offset, 3, f"[{'X' if checked[index] else ' '}] {addon['name']} {' ' * (10-len(addon['name']))} {addon['description']}", curses.A_REVERSE if index + 2 == current else 0)
                index += 1
            line_offset += 1

        stdscr.refresh()

    def draw_info(stdscr, element):
        stdscr.clear()
        stdscr.addstr(0, 0, f"Info for {element['name']}", curses.A_BOLD)
        stdscr.addstr(1, 0, f"{element['description']}")
        stdscr.addstr(3, 0, "Files to install:", curses.A_BOLD)
        for i, file in enumerate(element["files"]):
            stdscr.addstr(4 + i, 0, f"  {file['name']}{' ' * (max(0, 15 - len(file['name'])))} {domain(file['url'])}")
        stdscr.addstr(5 + len(element["files"]), 0, "Press any key to continue")
        stdscr.refresh()
        stdscr.getch()

    def download_selected(stdscr, checked):
        stdscr.clear()
        stdscr.addstr(0, 0, "Installing selected addons...", curses.A_BOLD)
        stdscr.refresh()
        print("\n")
        for i, addon in enumerate(ALL_ADOONS):
            if checked[i]:
                download_addon(addon)

    while True:
        draw_menu(stdscr, checked, current)
        key = stdscr.getch()
        if key == ord("q"):
            break
        elif key == curses.KEY_DOWN:
            current = min(current + 1, len(ALL_ADOONS) + 1)
        elif key == curses.KEY_UP:
            current = max(current - 1, 0)
        elif key == curses.KEY_RIGHT:
            if current > 0:
                draw_info(stdscr, ALL_ADOONS[current - 2])
        elif key == curses.KEY_LEFT:
            current = 0
        elif key == 10 or key == ord(" "):
            if current == 1:
                if any(checked):
                    checked = [False] * len(ALL_ADOONS)
                else:
                    checked = [True] * len(ALL_ADOONS)
            elif current == 0:
                download_selected(stdscr, checked)
                break
            else:
                checked[current - 2] = not checked[current - 2]
        elif key == ord("v"):
            download_selected(stdscr, checked)
            break

    # restore terminal
    curses.endwin()

#######################################################
#######################################################

def show_help():
    print(
        "USAGE: python3 get_addons.py [options|addon]",
        "OPTIONS:",
        "  -h: show this help",
        "  -g: graphic menu",
        "  -l: list available addons",
        "  -a: get all addons",
        "  -w: get all weighty addons",
        "ADDONS:"
    )

    for category in ADDONS:
        print(f"  {category}:")
        for addon in ADDONS[category]:
            print(f"    {addon}: {ADDONS[category][addon]['description']}")

    print(
        "EXAMPLES:"
        "  python3 get_addons.py -g"
        "  python3 get_addons.py -a"
        "  python3 get_addons.py lua"
    )

def show_list():
    print("Available addons:")
    for category in ADDONS:
        print(f"  {category}:")
        for addon in ADDONS[category]:
            print(f"    {addon}: {ADDONS[category][addon]['description']}")

table = {
    "-h": show_help,
    "-l": show_list,
    "-a": lambda: [download_addon(addon) for addon in [get_addons(name) for name in RECOMMENDED]],
    "-w": lambda: [download_addon(addon) for addon in ALL_ADOONS],
    "-g": lambda: curses.wrapper(graphic_menu)
}

#######################################################
#######################################################

# main

if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) == 0:
        print("ERROR: No addon specified (use -h for help or -g for graphic menu)")
        sys.exit(1)
    if args[0][0] == "-":
        if args[0] in table:
            try:
                table[args[0]]()
            except KeyboardInterrupt:
                print("-- Aborted by user")
        else:
            print("ERROR: Unknown option:", args[0])
    else:
        download_addon(args[0])
