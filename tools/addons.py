import urllib.request as urlreq
import curses, os, sys

path = os.path.dirname(os.path.abspath(__file__))
profan_path = path.rsplit(os.sep, 1)[0]

#######################################################
#######################################################

ADDONS = {
    "lua": {
        "description": "port of the Lua interpreter",
        "files": [
            {
                "name": "lua",
                "url": "https://github.com/elydre/lua-profan/releases/download/latest/lua.bin",
                "path": [profan_path, "out", "zapps", "fatpath", "lua.bin"]
            },
        ]
    },
    "doom": {
        "description": "port of the Doom game for profanOS",
        "files": [
            {
                "name": "doom",
                "url": "https://github.com/elydre/doom-profan/releases/download/latest/doom.bin",
                "path": [profan_path, "out", "zapps", "fatpath", "doom.bin"]
            },
            {
                "name": "doom1.wad",
                "url": "https://distro.ibiblio.org/slitaz/sources/packages/d/doom1.wad",
                "path": [profan_path, "out", "zada", "doom", "DOOM1.WAD"]
            }
        ]
    },
    "sulfur": {
        "description": "official sulfur language interpreter",
        "files": [
            {
                "name": "sulfur",
                "url": "https://github.com/elydre/sulfur_lang/releases/download/latest/sulfur-profanOS-i386.bin",
                "path": [profan_path, "out", "zapps", "fatpath", "sulfur.bin"]
            },
        ]
    },
    "tcc": {
        "description": "Tiny C Compiler port for profanOS",
        "files": [
            {
                "name": "tcc",
                "url": "https://github.com/elydre/tinycc-profan/releases/download/latest/tcc.bin",
                "path": [profan_path, "out", "zapps", "fatpath", "tcc.bin"]
            },
            {
                "name": "libtcc.a",
                "url": "https://github.com/elydre/tinycc-profan/releases/download/latest/libtcc.a",
                "path": [profan_path, "out", "sys", "libtcc.a"]
            },
        ]
    },
    "vlink": {
        "description": "vlink linker with multi-format support",
        "files": [
            {
                "name": "vlink",
                "url": "https://github.com/elydre/vlink-profan/releases/download/latest/vlink.bin",
                "path": [profan_path, "out", "zapps", "fatpath", "vlink.bin"]
            },
        ]
    }
}

WADDONS = {
    "halfix": {
        "description": "port of the Halfix x86 emulator for profanOS",
        "files": [
            {
                "name": "halfix",
                "url": "https://github.com/elydre/halfix-profan/releases/download/latest/halfix.bin",
                "path": [profan_path, "out", "zapps", "fatpath", "halfix.bin"]
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
    }
}

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

def get_addon(name: str) -> bool:
    all_addons = {**ADDONS, **WADDONS}

    if name not in all_addons:
        print("\rERROR: Unknown addon:", name)
        return False

    print(f"\rInstall {name.upper()}: {all_addons[name]['description']}")
    for sub in all_addons[name]["files"]:
        print(f"\r Getting {name} part: {sub['name']}")
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

    checked = [[False] * len(ADDONS)] + [[False] * len(WADDONS)]
    current = 0

    def draw_menu(stdscr, checked, current):
        stdscr.clear()
        stdscr.addstr(0, 0, "Select addons to install with ENTER", curses.A_BOLD)
        stdscr.addstr(1, 0, "Q: cancel, RIGHT: info, V: validate")

        stdscr.addstr(3, 1, "Download Selected" if any(checked[0]) or any(checked[1]) else "Exit without downloading", curses.A_REVERSE if current == 0 else 0)
        stdscr.addstr(4, 1, "Unselect all" if any(checked[0]) or any(checked[1]) else "Select all", curses.A_REVERSE if current == 1 else 0)

        stdscr.addstr(6, 0, "Addons:", curses.A_BOLD)
        for i, addon in enumerate(ADDONS):
            stdscr.addstr(7 + i, 1, f" [{'X' if checked[0][i] else ' '}] {addon.upper()}\t {ADDONS[addon]['description']}", curses.A_REVERSE if current == i + 2 else 0)

        stdscr.addstr(8 + len(ADDONS), 0, "Weighty addons:", curses.A_BOLD)
        for i, addon in enumerate(WADDONS):
            stdscr.addstr(9 + len(ADDONS) + i, 1, f" [{'X' if checked[1][i] else ' '}] {addon.upper()}\t {WADDONS[addon]['description']}", curses.A_REVERSE if current == i + 2 + len(ADDONS) else 0)

        stdscr.refresh()

    def draw_info(stdscr, element):
        data = ADDONS[element] if element in ADDONS else WADDONS[element]
        stdscr.clear()
        stdscr.addstr(0, 0, f"Info for {element.upper()}", curses.A_BOLD)
        stdscr.addstr(1, 0, f"{data['description']}")
        stdscr.addstr(3, 0, "Files to install:", curses.A_BOLD)
        for i, file in enumerate(data["files"]):
            stdscr.addstr(4 + i, 0, f"  {file['name']}{' ' * (max(0, 15 - len(file['name'])))} {domain(file['url'])}")
        stdscr.addstr(5 + len(data["files"]), 0, "Press any key to continue")
        stdscr.refresh()
        stdscr.getch()

    def download_selected(stdscr, checked):
        stdscr.clear()
        stdscr.addstr(0, 0, "Installing selected addons...", curses.A_BOLD)
        stdscr.refresh()
        print("\n")
        for i, addon in enumerate(ADDONS):
            if checked[0][i]:
                get_addon(addon)
        for i, addon in enumerate(WADDONS):
            if checked[1][i]:
                get_addon(addon)

    while True:
        draw_menu(stdscr, checked, current)
        key = stdscr.getch()
        if key == ord("q"):
            break
        elif key == curses.KEY_DOWN:
            current = min(current + 1, len(ADDONS) + len(WADDONS) + 1)
        elif key == curses.KEY_UP:
            current = max(current - 1, 0)
        elif key == curses.KEY_RIGHT:
            if current > 0:
                draw_info(stdscr, list(ADDONS.keys())[current - 2] if current < len(ADDONS) + 2 else list(WADDONS.keys())[current - len(ADDONS) - 2])
        elif key == curses.KEY_LEFT:
            current = 0
        elif key == 10 or key == ord(" "):
            if current == 1:
                if any(checked[0]) or any(checked[1]):
                    checked = [[False] * len(ADDONS)] + [[False] * len(WADDONS)]
                else:
                    checked = [[True] * len(ADDONS)] + [[True] * len(WADDONS)]
            elif current == 0:
                download_selected(stdscr, checked)
                break
            elif current < len(ADDONS) + 2:
                checked[0][current - 2] = not checked[0][current - 2]
            else:
                checked[1][current - len(ADDONS) - 2] = not checked[1][current - len(ADDONS) - 2]
        elif key == ord("v"):
            download_selected(stdscr, checked)
            break

    # restore terminal
    curses.endwin()

#######################################################
#######################################################

def show_help():
    msg = [
           "USAGE: python3 get_addons.py [options|addon]",
           "OPTIONS:",
           "  -h: show this help",
           "  -g: graphic menu",
           "  -l: list available addons",
           "  -a: get all addons",
           "  -w: get all weighty addons",
           "ADDONS:",
    ]

    for addon in ADDONS:
        msg += [f"  {addon}: {ADDONS[addon]['description']}"]

    msg += ["WEIGHTY ADDONS:"]
    for addon in WADDONS:
        msg += [f"  {addon}: {WADDONS[addon]['description']}"]

    msg += ["EXAMPLES:",
           "  python3 get_addons.py -g",
           "  python3 get_addons.py -a",
           "  python3 get_addons.py lua",
    ]

    print("\n".join(msg))

def show_list():
    print("Available addons:")
    for addon in ADDONS:
        print(f"  {addon}: {ADDONS[addon]['description']}")
    print("Weighty addons:")
    for addon in WADDONS:
        print(f"  {addon}: {WADDONS[addon]['description']}")

table = {
    "-h": show_help,
    "-l": show_list,
    "-a": lambda: [get_addon(addon) for addon in ADDONS],
    "-w": lambda: [get_addon(addon) for addon in [*ADDONS, *WADDONS]],
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
        get_addon(args[0])
