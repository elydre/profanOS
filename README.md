# profanOS

[![Makefile CI](https://github.com/elydre/profanOS/actions/workflows/makefile.yml/badge.svg)](https://github.com/elydre/profanOS/actions/workflows/makefile.yml)
[![nbr](https://img.shields.io/github/directory-file-count/esolangs/profanOS-build/img?label=release)](https://github.com/esolangs/profanOS-build/tree/main/img)
[![pwp](https://img.shields.io/badge/dynamic/json?color=blue&label=started%20pwp&query=count&url=https://elydre.github.io/build/count.json)](https://elydre.github.io/profan)
[![views](https://komarev.com/ghpvc/?username=profanOS&label=views)](https://github.com/elydre/profanOS)

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
apt-get install -y gcc nasm make qemu-system-i386 python3
```

### Compile & Run

```bash
# Simple compilation
make

# Compile and run
make run

# Show all commands
make info
```

You can also download the build images from the repo [profanOS-build](https://github.com/esolangs/profanOS-build/tree/main/img)
or the [latest release](https://github.com/elydre/profanOS/releases/tag/latest)

## OS documentation

### Command line

| command | description                     |
|---------|---------------------------------|
| alloc   | allocate *suffix* octets        |
| clear   | clear the screen                |
| echo    | print the arguments             |
| free    | free *suffix* address           |
| help    | show this help                  |
| info    | show time, task & page info     |
| mem     | show MLIST with colors          |
| reboot  | reboot the system               |
| sc      | show the scancodes              |
| sleep   | sleep for a given time          |
| ss      | show int32 in the LBA *suffix*  |
| stop    | shutdown the system             |
| td      | test the disk                   |
| usg     | show the usage of cpu           |
| ver     | display the version             |
| yield   | yield to pid *suffix*           |

### Keyboard

| key   | function               |
|-------|------------------------|
| lshft | maj (hold)             |
| rshft | paste the last command |

### Smart print color

| keys | light colors | keys | dark color   |
|------|--------------|------|--------------|
| `$0` | blue         | `$8` | dark blue    |
| `$1` | green        | `$9` | dark green   |
| `$2` | cyan         | `$A` | dark cyan    |
| `$3` | red          | `$B` | dark red     |
| `$4` | magenta      | `$C` | dark magenta |
| `$5` | yellow       | `$D` | dark yellow  |
| `$6` | grey         | `$E` | dark grey    |
| `$7` | white        | `$F` | black        |

### Interrupts

|  ID  | description                   |
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

## source and acknowledgment

* [os tutorial](https://github.com/cfenollosa/os-tutorial) for the original tutorial
* [fabian](https://github.com/copy/v86) for the v86 online emulator and floppy build
* [framindmap](https://framindmap.org) for the mindmap of the todo list
* [yuukidesu9](https://gitlab.com/yuukidesu9/yuuos) for the iso creation
* [szhou42](https://github.com/szhou42/osdev) for the ata driver
* [osdev](https://wiki.osdev.org/Cooperative_Multitasking) for the multitasking

*be careful with our friend 55*
