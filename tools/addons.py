import urllib.request as urlreq
import curses, os, sys

path = os.path.dirname(os.path.abspath(__file__))
profan_path = path.rsplit(os.sep, 1)[0]

#######################################################
#######################################################

RECOMMENDED = ["tcc", "lua", "zlib", "doom"]

FILEARRAY = [
    {
        "name": "tcc.elf",
        "url": "https://github.com/elydre/tinycc-profan/releases/download/latest/tcc.elf",
        "path": [profan_path, "out", "zapps", "fatpath", "tcc.elf"]
    },
    {
        "name": "libtcc.a",
        "url": "https://github.com/elydre/tinycc-profan/releases/download/latest/libtcc.a",
        "path": [profan_path, "out", "zlibs", "libtcc.a"]
    },
    {
        "name": "vlink.elf",
        "url": "https://github.com/elydre/vlink-profan/releases/download/latest/vlink.elf",
        "path": [profan_path, "out", "zapps", "fatpath", "vlink.elf"]
    },
    {
        "name": "libm.so",
        "url": "https://github.com/elydre/libs-profan/releases/download/latest/libm.so",
        "path": [profan_path, "out", "zlibs", "libm.so"]
    },
    {
        "name": "math.h",
        "url": "https://raw.githubusercontent.com/elydre/libs-profan/main/_headers/libm/math.h",
        "path": [profan_path, "include", "addons", "math.h"]
    },
    {
        "name": "complex.h",
        "url": "https://raw.githubusercontent.com/elydre/libs-profan/main/_headers/libm/complex.h",
        "path": [profan_path, "include", "addons", "complex.h"]
    },
    {
        "name": "fenv.h",
        "url": "https://raw.githubusercontent.com/elydre/libs-profan/main/_headers/libm/fenv.h",
        "path": [profan_path, "include", "addons", "fenv.h"]
    },
    {
        "name": "libm.h",
        "url": "https://raw.githubusercontent.com/elydre/libs-profan/main/_headers/libm/libm.h",
        "path": [profan_path, "include", "addons", "libm.h"]
    },
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
        "name": "gzip.elf",
        "url": "https://github.com/elydre/libs-profan/releases/download/latest/gzip.elf",
        "path": [profan_path, "out", "zapps", "cmd", "gzip.elf"]
    },
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
    {
        "name": "dash.elf",
        "url": "https://github.com/elydre/dash-profan/releases/download/latest/dash.elf",
        "path": [profan_path, "out", "zapps", "fatpath", "dash.elf"]
    },
    {
        "name": "lish.elf",
        "url": "https://github.com/elydre/libs-profan/releases/download/latest/lish.elf",
        "path": [profan_path, "out", "zapps", "fatpath", "lish.elf"]
    },
    {
        "name": "doom.elf",
        "url": "https://github.com/elydre/doom-profan/releases/download/latest/doom.elf",
        "path": [profan_path, "out", "zapps", "fatpath", "doom.elf"]
    },
    {
        "name": "doom1.wad",
        "url": "https://distro.ibiblio.org/slitaz/sources/packages/d/doom1.wad",
        "path": [profan_path, "out", "zada", "doom", "DOOM1.WAD"]
    },
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
        "name": "halfix.cfg",
        "url": "https://raw.githubusercontent.com/elydre/halfix-profan/master/default.conf",
        "path": [profan_path, "out", "zada", "halfix", "default.cfg"]
    },
    {
        "name": "linux.iso",
        "url": "https://github.com/copy/images/raw/master/linux.iso",
        "path": [profan_path, "out", "zada", "halfix", "linux.iso"]
    },
    {
        "name": "lua.elf",
        "url": "https://github.com/elydre/lua-profan/releases/download/latest/lua.elf",
        "path": [profan_path, "out", "zapps", "fatpath", "lua.elf"]
    },
    {
        "name": "sulfur.elf",
        "url": "https://github.com/elydre/sulfur_lang/releases/download/latest/sulfur-profanOS-i386.elf",
        "path": [profan_path, "out", "zapps", "fatpath", "sulfur.elf"]
    }
]

ADDONS = {
    "compilation tools": [
        {
            "name": "tcc",
            "description": "Small and fast C compiler",
            "files": ["tcc.elf", "libtcc.a", "libm.so"]
        },
        {
            "name": "vlink",
            "description": "multi-target linker [deprecated]",
            "files": ["vlink.elf"]
        },
    ],
    "libraries": [
        {
            "name": "libm",
            "description": "Math library",
            "files": ["libm.so", "libm.h", "math.h", "complex.h", "fenv.h"]
        },
        {
            "name": "zlib",
            "description": "Compression library + gzip command",
            "files": ["libz.so", "zlib.h", "gzip.elf"]
        },
        {
            "name": "libupng",
            "description": "Small PNG Decoding Library",
            "files": ["libupng.so", "upng.h"]
        }
    ],
    "extra shells": [
        {
            "name": "dash",
            "description": "POSIX compliant shell [experimental]",
            "files": ["dash.elf"]
        },
        {
            "name": "lish",
            "description": "Lightweight bash-like shell",
            "files": ["lish.elf"]
        }
    ],
    "graphics": [
        {
            "name": "doom",
            "description": "Raycasting first person shooter",
            "files": ["doom.elf", "doom1.wad", "libm.so"]
        },
        {
            "name": "halfix",
            "description": "x86 emulator with provided linux image",
            "files": ["halfix", "bios.bin", "vgabios.bin", "halfix.cfg", "linux.iso"]
        },
    ],
    "interpreters": [
        {
            "name": "lua",
            "description": "Lightweight scripting language",
            "files": ["lua.elf", "libm.so"]
        },
        {
            "name": "sulfur",
            "description": "Bytecode high-performance language",
            "files": ["sulfur.elf", "libm.so"]
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

def get_file(name: str) -> dict:
    for file in FILEARRAY:
        if file["name"] == name:
            return file
    return None

#######################################################
#######################################################

def domain(url: str) -> str:
    return url.split("/")[2]

def download(url: str, path: str) -> bool:
    try:
        with urlreq.urlopen(url) as response:
            with open(path, "wb") as file:
                file.write(response.read())
    except Exception as e:
        print(f"ERROR: {e}")
        return False

    return True

def download_addons(addons: list) -> bool:
    required = list(set([file for addon in addons for file in addon["files"]]))
    for file in required:
        e = get_file(file)
        if e is None:
            print(f"ERROR: File {file} not found")
            return False

        print(f"\r Getting {file}")

        # check if parent directory exists
        parent = os.sep.join(e["path"][:-1])
        if not os.path.exists(parent):
            os.makedirs(parent)

        # download
        if not download(e["url"], os.sep.join(e["path"])): return False

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
        try:
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
        except curses.error:
            curses.endwin()
            print("ERROR: Terminal too small to display menu")
            sys.exit(1)

        stdscr.refresh()

    def draw_info(stdscr, element):
        stdscr.clear()
        stdscr.addstr(0, 0, f"Info for {element['name']}", curses.A_BOLD)
        stdscr.addstr(1, 0, f"{element['description']}")
        stdscr.addstr(3, 0, "Files to install:", curses.A_BOLD)
        for i, file in enumerate(element["files"]):
            e = get_file(file)
            stdscr.addstr(4 + i, 0, f"  {file}{' ' * (max(0, 15 - len(file)))} {domain(e['url'])}")
        stdscr.addstr(5 + len(element["files"]), 0, "Press any key to continue")
        stdscr.refresh()
        stdscr.getch()

    def download_selected(stdscr, checked):
        stdscr.clear()
        stdscr.addstr(0, 0, "Installing selected addons...", curses.A_BOLD)
        stdscr.refresh()
        print("\n")
        requested = [ALL_ADOONS[i] for i in range(len(ALL_ADOONS)) if checked[i]]
        download_addons(requested)

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
    "-a": lambda: [download_addons([get_addons(e) for e in RECOMMENDED])],
    "-w": lambda: [download_addons(ALL_ADOONS)],
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
        download_addons([get_addons(args[0])])
