# profanOS

[![Makefile CI](https://github.com/elydre/profanOS/actions/workflows/makefile.yml/badge.svg)](https://github.com/elydre/profanOS/actions/workflows/makefile.yml)
[![download](https://img.shields.io/github/size/esolangs/profanOS-build/profanOS.iso?label=download%20iso)](https://github.com/esolangs/profanOS-build/blob/main/profanOS.iso)
![views](https://komarev.com/ghpvc/?username=profanOS&label=views)
![lines](https://tokei.rs/b1/github/elydre/profanOS)

![wave](https://elydre.github.io/img/profan.svg)

This repo originally comes from part 23 of the excellent tutorial [How to create an OS from scratch](https://github.com/cfenollosa/os-tutorial) thanks!

You can find the list of things to do [here](https://framindmap.org/c/maps/1263862/embed)

## Setup

> **Note** -
> compilation is guaranteed only on debian & co, but
> it is also possible in windows with virtualization
> solutions like wsl (on windows 11) or hyperV

### Install dependencies

```bash
sudo apt-get install -y gcc nasm make qemu-system-i386 genisoimage python3
```

### Compile & Run

```bash
# Simple compilation
make

# Compile and run
make run

# Show all commands
make help
```

You can also [download the iso](https://github.com/esolangs/profanOS-build/raw/main/profanOS.iso) from the repo [profanOS-build](https://github.com/esolangs/profanOS-build)

## OS documentation

### Command line

| Command | Description                  |
|---------|------------------------------|
| CLEAR   | clear the screen             |
| ECHO    | print the argument           |
| END     | shutdown the system          |
| HELP    | show the help                |
| ISR     | test interrupt handler       |
| PAGE    | show the actual page address |
| TASK    | print info about the tasks   |
| TD      | test the disk                |
| VER     | display the version          |
| YIELD   | yield to the next task       |

### Keyboard

| Key   | Function               |
|-------|------------------------|
| esc   | entering scancode mode |
| lshft | maj (hold)             |
| rshft | paste the last command |

### Interrupts

|  ID  | Description                   |
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

### Tasking rotation

```
> t1 t2 t3 .
- t1 t2 t3 t3
- t1 t2 t2 t3
- t1 t1 t2 t3
> t3 t1 t2 t3
```

## Author & Contact

* [pf4](https://github.com/elydre)
* [loris](https://github.com/Lorisredstone)

Contact us on [discord](https://pf4.ddns.net/discord)

## source and acknowledgment

* [os tutorial](https://github.com/cfenollosa/os-tutorial) for the original tutorial
* [framindmap](https://framindmap.org) for the mindmap of the todo list
* [yuukidesu9](https://gitlab.com/yuukidesu9/yuuos) for the iso creation
* [szhou42](https://github.com/szhou42/osdev) for the ata driver
* [osdev](https://wiki.osdev.org/Cooperative_Multitasking) for the multitasking
