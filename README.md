# profanOS

[![Makefile CI](https://github.com/elydre/profanOS/actions/workflows/makefile.yml/badge.svg)](https://github.com/elydre/profanOS/actions/workflows/makefile.yml)
[![nbr](https://img.shields.io/github/commit-activity/m/elydre/profanOS)](https://github.com/esolangs/profanOS-build/tree/main/img)
[![lines](https://img.shields.io/badge/dynamic/json?color=blue&label=code%20lines&query=profan_lines&url=https://elydre.github.io/build/count.json)](https://elydre.github.io/profan)
[![test](https://img.shields.io/badge/click%20to%20test-latest-blue)](https://elydre.github.io/profan/latest)

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
sudo apt-get update
sudo apt-get install -y gcc g++ nasm make qemu-system-i386 python3 grub-common xorriso grub-pc-bin mtools
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

You can also download the build images from the repo [profanOS-build](https://github.com/esolangs/profanOS-build)
or the [latest release](https://github.com/elydre/profanOS/releases/tag/latest)

## OS documentation

### shell.bin help

```
cat     - print file *              mem     - show MLIST with colors
cd      - change dir to *           mkdir   - create a new folder *
clear   - clear the screen          mkfile  - create a new file *
echo    - print the arguments       reboot  - reboot the system
exit    - exit of the shell app     sc      - show the scancodes
go      - start * file as binary    sleep   - sleep for a given time
help    - show this help            stop    - shutdown the system
info    - show time, task & page    udisk   - show used disk space
ls      - list the current dir
```

## Known major bugs

| bug name  | exisiting since | description | fix idea |
|-----------|-----------------|-------------|----------|
| laged lag | ? | parts of profanOS slow<br>down at times on qemu | - memory address problem?<br>- qemu problem?|

## Author & Contact

* pf4 ([@elydre](https://github.com/elydre))
* Loris ([@Lorisredstone](https://github.com/Lorisredstone))
* ProtoByter ([@ProtoByter](https://github.com/ProtoByter))

Contact us on [discord](https://pf4.ddns.net/discord)

## Source & Acknowledgment

* [os tutorial](https://github.com/cfenollosa/os-tutorial) for the original tutorial
* [fabian](https://github.com/copy/v86) for the v86 online emulator and floppy build
* [framindmap](https://framindmap.org) for the mindmap of the todo list
* [yuukidesu9](https://gitlab.com/yuukidesu9/yuuos) for the iso creation
* [szhou42](https://github.com/szhou42/osdev) for the ata driver
* [osdev](https://wiki.osdev.org/Cooperative_Multitasking) for the multitasking

*be careful with our friend 55*
