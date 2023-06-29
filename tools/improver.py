import os

# scan file and remove trailing whitespace
def scan_file(path):
    contant = ""
    with open(path) as f:
        for l, c in enumerate(f, 1):
            # check if line only contains whitespace
            if c.isspace() and len(c) > 1:
                print(f"{path}:{l} contains {len(c) - 1} whitespace characters")
                contant += "\n"
            else:
                contant += c

    with open(path, "w") as f:
        f.write(contant)
                
# scan directory for files and directories
def scan_dir(path):
    # list files in directory
    for f in os.listdir(path):
        name = os.path.join(path, f)

        if os.path.isfile(name) and name.endswith(".c"):
            scan_file(name)

        elif os.path.isdir(name):
            scan_dir(name)

# run script
if __name__ == "__main__":
    scan_dir(".")
