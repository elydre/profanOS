import urllib.request as urlreq
import os, sys

# setup

path = os.path.dirname(os.path.abspath(__file__))
profan_path = path.rsplit(os.sep, 1)[0]

ADDONS = {
    "lua": {
        "description": "port of the Lua interpreter for profanOS",
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
        ]
    },
    "vlink": {
        "description": "vlink linker port for profanOS",
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

# functions

def get_size(url: str) -> int:
    try: return int(urlreq.urlopen(url).info()["Content-Length"]) // 1024
    except Exception: return 0

def download(url: str, path: str) -> bool:
    def show_progress(block_num, block_size, total_size):
        percent = int(block_num * block_size * 100 / total_size)
        print(f" | Downloaded {percent}%", end="\r")

    # get file
    try:
        urlreq.urlretrieve(url, path, show_progress)
    except Exception:
        print(" | ERROR: Could not download file")
        return False

    return True

def get_addon(name: str) -> bool:
    all_addons = {**ADDONS, **WADDONS}

    if name not in all_addons:
        print("ERROR: Unknown addon:", name)
        return False

    print(f"Install {name.upper()}: {all_addons[name]['description']}")
    for sub in all_addons[name]["files"]:
        print(f" Getting {name} part: {sub['name']} ({get_size(sub['url'])}Ko)")
        # check if parent directory exists
        parent = os.sep.join(sub["path"][:-1])
        if not os.path.exists(parent):
            print(" | Creating directory", parent)
            os.makedirs(parent)

        print(f" | Downloading {sub['url']}")

        # download
        if not download(sub["url"], os.sep.join(sub["path"])): return False

    return True

def show_help():
    msg = [
           "USAGE: python3 get_addons.py [options] [addon]",
           "OPTIONS:",
           "  -h: show this help",
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
           "  python3 get_addons.py -l",
           "  python3 get_addons.py -a",
           "  python3 get_addons.py lua",
    ]

    print("\n".join(msg))

def show_list():
    print("Available addons:")
    for addon in ADDONS:
        print(f"  {addon}: {ADDONS[addon]['description']}")

table = {
    "-h": show_help,
    "-l": show_list,
    "-a": lambda: [get_addon(addon) for addon in ADDONS],
    "-w": lambda: [get_addon(addon) for addon in [*ADDONS, *WADDONS]]
}

# main

if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) == 0:
        print("ERROR: No addon specified (use -h for help)")
        sys.exit(1)
    if args[0][0] == "-":
        if args[0] in table:
            table[args[0]]()
        else:
            print("ERROR: Unknown option:", args[0])
    else:
        get_addon(args[0])
