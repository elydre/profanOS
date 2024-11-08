#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
#   === addons.py : 2024 ===                                                  #
#                                                                             #
#    Python script to download and install addons for profanOS     .pi0iq.    #
#                                                                 d"  . `'b   #
#    This file is part of profanOS and is released under          q. /|\  "   #
#    the terms of the GNU General Public License                   `// \\     #
#                                                                  //   \\    #
#   === elydre : https://github.com/elydre/profanOS ===         #######  \\   #
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

import urllib.request as urlreq
import curses, os, sys, json

path = os.path.dirname(os.path.abspath(__file__))
profan_path = path.rsplit(os.sep, 1)[0]

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

def json_from_url(url):
    with urlreq.urlopen(url) as response:
        data = response.read()
    return json.loads(data)

try:
    addons_json = json_from_url("https://elydre.github.io/profan/addons.json")
except Exception as e:
    print("Failed to retrieve JSON data from URL:", e)
    sys.exit(1)

RECOMMENDED = addons_json["RECOMMENDED"]
ADDONS = addons_json["ADDONS"]
FILEARRAY = addons_json["FILEARRAY"]

for e in FILEARRAY:
    e["path"] = [profan_path] + e["make_path"]

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

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

def domain(url: str) -> str:
    return url.split("/")[2]

def download(url: str, path: str):
    with urlreq.urlopen(url) as response:
        with open(path, "wb") as file:
            file.write(response.read())

def download_targz(url: str, path: str):
    targz = path + ".tar.gz"

    download(url, targz)

    if os.path.exists(path):
        os.remove(path)
    os.makedirs(path)

    os.system(f"tar -xf {targz} -C {path}")
    os.remove(targz)

def download_addons(addons: list) -> bool:
    required = list(set([file for addon in addons for file in addon["files"]]))
    required.sort()

    for file in required:
        e = get_file(file)
        if e is None:
            print(f"\rERROR: File {file} not found")
            return False

        print(f"\r Getting {file}")

        # check if parent directory exists
        parent = os.sep.join(e["path"][:-1])
        if not os.path.exists(parent):
            os.makedirs(parent)

        # download
        try:
            if "is_targz" in e.keys() and e["is_targz"]:
                download_targz(e["url"], os.sep.join(e["path"]))
            else:
                download(e["url"], os.sep.join(e["path"]))
        except Exception as e:
            print(f"\rERROR: {e}")
            return False
    return True

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

def graphic_menu(stdscr: curses.window):
    curses.curs_set(0)
    stdscr.clear()
    stdscr.refresh()


    checked = [False] * len(ALL_ADOONS)

    print(len(ALL_ADOONS))

    current = 1

    def draw_ifposible(stdscr, y, x, text, mode):
        max_y, max_x = stdscr.getmaxyx()
        if len(text) + x > max_x - 1:
            text = text[:max_x - x - 2] + "."
        if y < max_y and y >= 0:
            stdscr.addstr(y, x, text, mode)

    def get_current_y():
        index = 0
        line_offset = 6
        for category in ADDONS:
            line_offset += 1
            for _ in ADDONS[category]:
                if index + 2 == current:
                    return line_offset + index
                index += 1
            line_offset += 1
        return 0

    def draw_menu(stdscr, checked, current):
        yoffset = get_current_y()
        max_y, _ = stdscr.getmaxyx()

        if yoffset > max_y - 3:
            yoffset -= max_y - 3
        else:
            yoffset = 0

        stdscr.clear()
        draw_ifposible(stdscr, 0 - yoffset, 0, "Select addons to install with ENTER", curses.A_BOLD)
        draw_ifposible(stdscr, 1 - yoffset, 0, "Q: cancel, RIGHT: info, V: validate", 0)

        draw_ifposible(stdscr, 3 - yoffset, 1,
                       "Download selected and Exit" if any(checked) else "Exit without downloading",
                       curses.A_REVERSE if current == 0 else 0)
        draw_ifposible(stdscr, 4 - yoffset, 1, "Unselect all" if any(checked) else "Select recommended",
                       curses.A_REVERSE if current == 1 else 0)

        index = 0
        line_offset = 6
        for category in ADDONS:
            draw_ifposible(stdscr, index + line_offset - yoffset, 1, category.upper(), curses.A_BOLD)
            line_offset += 1
            for addon in ADDONS[category]:
                draw_ifposible(stdscr, index + line_offset - yoffset, 3,
                               f"[{'X' if checked[index] else ' '}] {addon['name']}" +
                               f"{' ' * (10-len(addon['name']))} {addon['description']}",
                               curses.A_REVERSE if index + 2 == current else 0)
                index += 1
            line_offset += 1

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
        if not download_addons(requested):
            stdscr.getch()

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
                    checked = [True if ALL_ADOONS[i]["name"] in RECOMMENDED else False for i in range(len(ALL_ADOONS))]
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

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

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

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

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
