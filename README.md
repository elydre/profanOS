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
> compilation is guaranteed only on ubuntu 22.04 with 
> gcc 11, it is also possible in windows with virtualization
> solutions like wsl (on windows 11) or hyperV

### Install dependencies

```bash
sudo apt-get update
sudo apt-get install -y gcc g++ nasm make qemu-system-i386 python3 grub-common xorriso grub-pc-bin mtools
```

### Compile & Run

```bash
# Simple compilation
make elf

# Compile and run
make run

# Show all commands
make
```

You can also download the build images from the repo [profanOS-build](https://github.com/esolangs/profanOS-build)
or the [latest release](https://github.com/elydre/profanOS/releases/tag/latest)

## OS documentation

### shell.bin help

```
binc    - elf to binary converter   info    - show time, mem, disk
cat     - show file content         ls      - list the current dir
cd      - change dir                mkdir   - create a new dir
clear   - clear the screen          mkfile  - create a new file
cpu     - start cpu usage monitor   proc    - show process info
echo    - print the arguments       reboot  - reboot the system
exit    - exit of the shell app     rim     - start text editor
go      - start * file as binary    stop    - shutdown the system
help    - show this help            udisk   - show used disk space
```

### Real-Hardware

profanOS works on pc with legacy bios but not with uefi. However profanOS
can work on recent pc by activating bios compatibility.

To install profanOS on a USB key or an internal disk, it is possible to use
the installation script `tools/install.sh` or any other image flasher.

> **Warning** -
> installing an OS on a real machine can be risky and
> must be done with knowledge of the possible risks


### Known major bugs

| bug name  | since | description | cause | fixed ? |
|-----------|-------|-------------|-------|---------|
| lagged lag | ? | all profanOS is getting very slow in qemu | memory | partially |
| BOBCAT | 0.4.2 | some version of gcc build broken zapps | gcc | no |

## Author & Contact

* pf4 ([@elydre](https://github.com/elydre))
* Loris ([@Lorisredstone](https://github.com/Lorisredstone))

Contact us on [discord](https://pf4.ddns.net/discord)

## Source & Acknowledgment

* [os tutorial](https://github.com/cfenollosa/os-tutorial) for the original tutorial
* [fabian](https://github.com/copy/v86) for the v86 online emulator and floppy build
* [framindmap](https://framindmap.org) for the mindmap of the todo list
* [yuukidesu9](https://gitlab.com/yuukidesu9/yuuos) for the iso creation
* [szhou42](https://github.com/szhou42/osdev) for the ata driver
* [osdev](https://wiki.osdev.org/Cooperative_Multitasking) for the multitasking
* ProtoByter ([@ProtoByter](https://github.com/ProtoByter)) for the first pull request

*be careful with our friend 55*
