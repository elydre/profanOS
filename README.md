# profanOS

[![Makefile CI](https://github.com/elydre/profanOS/actions/workflows/makefile.yml/badge.svg)](https://github.com/elydre/profanOS/actions/workflows/makefile.yml)
[![flawfinder](https://github.com/elydre/profanOS/actions/workflows/flawfinder.yml/badge.svg)](https://github.com/elydre/profanOS/actions/workflows/flawfinder.yml)
![views](https://komarev.com/ghpvc/?username=profanOS&color=aaaaaa&label=views)

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

| Key   | Function               |
|-------|------------------------|
| esc   | entering scancode mode |
| lshft | maj (hold)             |
| rshft | paste the last command |

## Interrupts

|  Id  | Description                   |
|------|-------------------------------|
|  00  | Division By Zero              |
|  01  | Debug                         |
|  02  | Non Maskable Interrupt        |
|  03  | Breakpoint                    |
|  04  | Into Detected Overflow        |
|  05  | Out of Bounds                 |
|  06  | Invalid Opcode                |
|  07  | No Coprocssor                 |
|  08  | Double Fault                  |
|  09  | Coprocessor Segment Overrun   |
|  10  | Bad TSS                       |
|  11  | Segment Not Present           |
|  12  | Stack Fault                   |
|  13  | General Protection Fault      |
|  14  | Page Fault                    |
|  15  | Unknown Interrupt             |
|  16  | Coprocessor Fault             |
|  17  | Alignment Check               |
|  18  | Machine Check                 |


## Author & Contact

* [pf4](https://github.com/elydre)
* [loris](https://github.com/Lorisredstone)

Contact us on [discord](https://pf4.ddns.net/discord)
