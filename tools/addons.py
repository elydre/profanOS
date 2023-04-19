import os
import shutil
import sys
import urllib.request

# setup

path = os.path.dirname(os.path.abspath(__file__))
profan_path = path.rsplit(os.sep, 1)[0]

ADDONS = {
    "lua": {
        "description": "port of the Lua interpreter for profanOS",
        "files": [
            {   
                "name": "lua-core",
                "url": "https://github.com/elydre/lua-profan/releases/download/latest/lua-core.bin",
                "path": [profan_path, "out", "zapps", "tools", "lua-core.bin"]
            },
            {
                "name": "lua-launcher",
                "url": "https://github.com/elydre/lua-profan/releases/download/latest/lua.bin",
                "path": [profan_path, "out", "zapps", "commands", "lua.bin"]
            },
        ]
    },
    "doom": {
        "description": "port of the Doom game for profanOS",
        "files": [
            {
                "name": "doom-core",
                "url": "https://github.com/elydre/doom-profan/releases/download/latest/doom-core.bin",
                "path": [profan_path, "out", "zapps", "tools", "doom-core.bin"]
            },
            {
                "name": "doom-launcher",
                "url": "https://github.com/elydre/doom-profan/releases/download/latest/doom.bin",
                "path": [profan_path, "out", "zapps", "commands", "doom.bin"]
            },
            {
                "name": "doom1.wad",
                "url": "https://distro.ibiblio.org/slitaz/sources/packages/d/doom1.wad",
                "path": [profan_path, "out", "zapps", "doom", "DOOM1.WAD"]
            }
        ]
    }
}

# functions

def download(url: str, path: str) -> bool:
    try:
        with urllib.request.urlopen(url) as response, open(path, "wb") as out_file:
            shutil.copyfileobj(response, out_file)
        return True
    except Exception as e:
        print(" | ERROR while downloading", url, ":", e)
    return False

def get_addon(name: str) -> bool:
    if name not in ADDONS:
        print("ERROR: Unknown addon:", name)
        return False
    
    print(f"Install {name.upper()}: {ADDONS[name]['description']}")
    for sub in ADDONS[name]["files"]:
        print(f" Getting {name} part: {sub['name']}")
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
           "ADDONS:",
    ]

    for addon in ADDONS:
        msg += [f"  {addon}: {ADDONS[addon]['description']}"]
    
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
