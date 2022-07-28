# profanOS

[![Makefile CI](https://github.com/elydre/profanOS/actions/workflows/makefile.yml/badge.svg)](https://github.com/elydre/profanOS/actions/workflows/makefile.yml)
![views](https://komarev.com/ghpvc/?username=profanOS&color=2db84d&label=views)


![wave](https://elydre.github.io/img/profan.svg)

This repo originally comes from part 23 of the excellent tutorial [How to create an OS from scratch](https://github.com/cfenollosa/os-tutorial) thanks!

You can find the list of things to do [here](https://framindmap.org/c/maps/1263862/embed)

## Install dependencies

```bash
sudo apt-get install -y gcc nasm make qemu-system-i386
```

## Compile & Run

```bash
# Simple compilation
make

# Compile and run
make run

# Clean the output
make clean

# Clean all
make fullclean
```

## Command line

| Command | Description            |
|---------|------------------------|
| CLEAR   | clear the screen       |
| ECHO    | print the argument     |
| END     | shutdown the system    |
| HELP    | show the help          |
| ISR     | test interrupt handler |
| TD      | test the disk          |
| VERSION | display the version    |

## Keyboard

| Key  | Function               |
|------|------------------------|
| esc  | Entering scancode mode |
| lshft| maj (hold)             |
| rshft| paste the last command |

## Author & Contact

* [pf4](https://github.com/elydre)
* [loris](https://github.com/Lorisredstone)

Contact us on [discord](https://pf4.ddns.net/discord)
