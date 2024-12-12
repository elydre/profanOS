#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
#   === improver.py : 2024 ===                                                #
#                                                                             #
#    Python script to normalize source code style                  .pi0iq.    #
#                                                                 d"  . `'b   #
#    This file is part of profanOS and is released under          q. /|\  "   #
#    the terms of the GNU General Public License                   `// \\     #
#                                                                  //   \\    #
#   === elydre : https://github.com/elydre/profanOS ===         #######  \\   #
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

import os, sys, time

scan_externtions = [
    ".py",  ".sh",
    ".c",   ".h",
    ".cpp", ".hpp",
    ".txt", ".md",
    ".asm", ".s",
    ".ld",
]

ignore_directories = [
    "out",
    "include/addons",
    "sys_dir/user",
]

ignore_line_too_long = [
    ".md", ".h"
]

file_with_header = [
    ".c",   ".h",
    ".cpp", ".hpp",
    ".ld"
]

analyzed = {
    "files": 0,
    "lines": 0,
    "patch": 0,
    "unpatch": 0,
    "l_n": "",
    "l_w": "",
}

C_NOTE = "\033[92m"
C_WARNING = "\033[93m"
C_END = "\033[0m"

empty_header = [
    "/*****************************************************************************\\\n",
    "|   ===                                                                       |\n",
    "|                                                                             |\n",
    "|    -                                                             .pi0iq.    |\n",
    "|                                                                 d\"  . `'b   |\n",
    "|    This file is part of profanOS and is released under          q. /|\\  \"   |\n",
    "|    the terms of the GNU General Public License                   `// \\\\     |\n",
    "|                                                                  //   \\\\    |\n",
    "|   === elydre : https://github.com/elydre/profanOS ===         #######  \\\\   |\n",
    "\\*****************************************************************************/\n"
]

def print_note(file, line, message):
    line += 1
    analyzed["patch"] += 1
    if instant_print:
        print(f"{C_NOTE}    {file}:{line} {message}{C_END}")
        return
    analyzed["l_n"] += f"{C_NOTE}    {file}:{line} {message}{C_END}\n"

def print_warning(file, line, message):
    line += 1
    analyzed["unpatch"] += 1
    if instant_print:
        print(f"{C_WARNING}    {file}:{line} {message}{C_END}")
        return
    analyzed["l_w"] += f"{C_WARNING}    {file}:{line} {message}{C_END}\n"

apply_patch = True
instant_print = False

def tab_to_spaces(line, tab_size=4):
    new_line = ""
    i = 0
    for c in line:
        if c == "\t":
            u = tab_size - i % tab_size
            new_line += " " * u
            i += u
        else:
            new_line += c
            i += 1
    return new_line

def add_header(lines, path):
    print_note(path, 1, "[fixed] added empty header")
    if len(lines) > len(empty_header):
        same = True
        for i in range(len(empty_header)):
            if lines[i][0] != empty_header[i][0]:
                same = False
                break
        if same:
            lines = lines[len(empty_header) + 1:]

    copy = empty_header.copy()
    name = f"{path.split('/')[-1]} : {time.strftime('%Y')}"
    copy[1] = "|   === " + name + " ===" + " " * (66 - len(name)) + "|\n"
    return copy + ["\n"] + lines

def check_for_header(lines, path):
    if len(lines) < 10:
        lines = add_header(lines, path)
    for l in [0, 2, 7, 8, 9]:
        if lines[l] != empty_header[l]:
            lines = add_header(lines, path)
            break
    if sum([e == "=" for e in lines[1]]) != 6 or not (path.split("/")[-1] in lines[1]):
        print_warning(path, 1, "no file name in header")
    if lines[3] == empty_header[3]:
        print_warning(path, 3, "no file description in header")
    elif not (lines[3][5].isupper() or lines[3][5].isnumeric() or lines[3][5:].startswith("profan")):
        print_warning(path, 3, "file description should start with uppercase")
    for l in range(len(empty_header)):
        if len(lines[l]) != len(empty_header[l]):
            print_warning(path, l, "Non-standard header length")
            continue
        for i in range(5):
            if lines[l][i] != empty_header[l][i]:
                print_warning(path, l, "Non-standard header start")
                break
        for i in range(-18, 0):
            if lines[l][i] != empty_header[l][i]:
                print_warning(path, l, "Non-standard header end")
                break

    return lines

def detecte_brace(line, prev_line, path, l):
    if line.strip() == "{" and prev_line is not None and not prev_line.strip() == "":
        if (len(line) - len(line.lstrip())) < (len(prev_line) - len(prev_line.lstrip())):
            return
        print_warning(path, l, "brace should not be on a new line")

# scan file and remove trailing whitespace
def scan_file(path):
    analyzed["files"] += 1
    contant = ""
    last_line = None
    with open(path) as f:
        lines = f.readlines()
        if os.path.splitext(path)[1] in file_with_header:
            lines = check_for_header(lines, path)
        for l, c in enumerate(lines):
            analyzed["lines"] += 1

            line = c[:-1] # remove newline

            # check if line contains tab
            if "\t" in line:
                if not apply_patch:
                    print_warning(path, l, "contains tab")
                else:
                    print_note(path, l, "[fixed] contains tab")
                    line = tab_to_spaces(line)

            # check if line ends with space
            if line.endswith(" "):
                if not apply_patch:
                    print_warning(path, l, "ends with whitespace")
                else:
                    print_note(path, l, "[fixed] ends with whitespace")
                    line = line.rstrip()

            detecte_brace(line, last_line, path, l)

            # warning if line is too long
            if len(line) > 120 and not os.path.splitext(path)[1] in ignore_line_too_long:
                print_warning(path, l, "line is too long")

            contant += line + "\n"
            last_line = line

    with open(path, "w") as f:
        f.write(contant)

# scan directory for files and directories
def scan_dir(path):
    if os.path.isfile(path):
        scan_file(path)
        return

    # list files in directory
    for f in os.listdir(path):
        name = os.path.join(path, f)
        if name.startswith("./"):
            name = name[2:]
        if name.startswith("."):
            continue

        if os.path.isfile(name):
            # check if file has good extension
            if os.path.splitext(name)[1] in scan_externtions:
                scan_file(name)

        elif os.path.isdir(name):
            if name in ignore_directories:
                continue
            scan_dir(name)

def parse_args(argv):
    path = None

    global apply_patch
    for arg in argv:
        if arg == "-s":
            apply_patch = False
        elif arg == "-i":
            instant_print = True
        elif arg == "-h":
            print("Usage: python3 improver.py [-s]")
            print("  -i: instant print, print issues as they are found")
            print("  -s: strict mode, only print issues without fixing them")
            sys.exit(0)
        elif arg[0] != "-" and path is None:
            path = arg
        else:
            print(f"Unknown argument: {arg}")
            sys.exit(1)

    if path is None:
        path = "."

    return path

def main(args):
    scan_dir(parse_args(args))
    if not instant_print:
        print(analyzed["l_n"], end="")
        print(analyzed["l_w"], end="")
    print(f"End of scan, {analyzed['lines']} lines analyzed in {analyzed['files']} files!")
    print(f"{analyzed['patch']} lines edited, {analyzed['unpatch']} issues found")
    sys.exit(0 if analyzed["unpatch"] == 0 else 2)

# run script
if __name__ == "__main__":
    main(sys.argv[1:])
