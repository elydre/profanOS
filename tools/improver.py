import os

good_externtions = [
    ".py",  ".sh",
    ".c",   ".h",
    ".cpp", ".hpp",
    ".txt", ".md",
    ".asm", ".s",
    ".ld",
]

# scan file and remove trailing whitespace
def scan_file(path):
    contant = ""
    with open(path) as f:
        for l, c in enumerate(f, 1):
            line = c[:-1] # remove newline

            # check if line ends with whitespace
            if line.endswith(" ") or line.endswith("\t"):
                print(f"{path}:{l} ends with whitespace")
                contant += c.rstrip() + "\n"

            else:
                contant += c

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
