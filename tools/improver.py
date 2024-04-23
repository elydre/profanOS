import os

good_externtions = [
    ".py",  ".sh",
    ".c",   ".h",
    ".cpp", ".hpp",
    ".txt", ".md",
    ".asm", ".s",
    ".ld",
]

analyzed = {
    "files": 0,
    "lines": 0,
    "patch": 0,
    "too_long": 0,
}

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

# scan file and remove trailing whitespace
def scan_file(path):
    analyzed["files"] += 1
    contant = ""
    with open(path) as f:
        for l, c in enumerate(f, 1):
            analyzed["lines"] += 1

            line = c[:-1] # remove newline

            # check if line contains tab
            if "\t" in line:
                analyzed["patch"] += 1
                print(f"{path}:{l} contains tab")
                line = tab_to_spaces(line)

            # check if line ends with space
            if line.endswith(" "):
                analyzed["patch"] += 1
                print(f"{path}:{l} ends with whitespace")
                line = line.rstrip()

            # warning if line is too long
            if len(line) > 120 and not path.endswith(".md") and not path.endswith(".h"):
                analyzed["too_long"] += 1
                print(f"{path}:{l} is too long")

            contant += line + "\n"

    with open(path, "w") as f:
        f.write(contant)

# scan directory for files and directories
def scan_dir(path):
    # list files in directory
    for f in os.listdir(path):
        name = os.path.join(path, f)

        if os.path.isfile(name):
            # check if file has good extension
            if os.path.splitext(name)[1] in good_externtions:
                scan_file(name)

        elif os.path.isdir(name):
            scan_dir(name)

# run script
if __name__ == "__main__":
    scan_dir(".")
    print(f"End of scan, {analyzed['lines']} lines analyzed in {analyzed['files']} files!")
    print(f"{analyzed['patch']} lines edited, {analyzed['too_long']} lines too long")
