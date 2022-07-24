# profanOS

![wave](https://elydre.github.io/img/profan.svg)

This repo originally comes from part 23 of the excellent tutorial [How to create an OS from scratch](https://github.com/cfenollosa/os-tutorial) thanks!

You can find the list of things to do [here](https://framindmap.org/c/maps/1263862/embed)

## Install dependencies

```bash
sudo apt-get install -y gcc nasm make qemu-system-i386
```

## Compile & run

```bash
# Simple compilation
make

# Compile and run
make run

# Clean the output
make clean
```

## Command line

| Command | Description         |
|---------|---------------------|
| CLEAR   | clear the screen    |
| ECHO    | print the argument  |
| END     | shutdown the system |
| HELP    | show the help       |
| TD      | test the disk       |

## keyboard

| Key  | Function               |
|------|------------------------|
| esc  | Entering scancode mode |
| lshft| caps lock like         |
| rshft| paste the last command |
