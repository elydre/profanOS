# The profan Operating System

[![Makefile CI](https://github.com/elydre/profanOS/actions/workflows/makefile.yml/badge.svg)](https://github.com/elydre/profanOS/actions/workflows/makefile.yml)
[![nbr](https://img.shields.io/github/commit-activity/m/elydre/profanOS)](https://github.com/esolangs/profanOS-build/tree/main/img)
[![lines](https://img.shields.io/badge/dynamic/json?color=blue&label=code%20lines&query=profan_lines&url=https://elydre.github.io/build/count.json)](https://elydre.github.io/profan)
[![latest](https://img.shields.io/badge/click%20to%20test-latest-blue)](https://elydre.github.io/profan/latest.html)

![wave](https://elydre.github.io/img/profan.svg)

The profan Operating System is an independent OS developed from scratch.
It is not intended to be used massively or to have broad hardware support.

profanOS is characterized by its ring0-only preemptive modular multitasking
minimalist kernel and colorful-looking command line-based user interface.

You can find a progress map [here](https://framindmap.org/c/maps/1263862/embed)

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
make iso

# Compile and run
make run

# Show all commands
make
```

Each time the disk is modified you must force its reconstruction with `make disk`.
The main ports (more information in the [ports](#major-ports) section) are not included
in the repo source code but are easily downloadable with `make addons disk`.

### Automated build

You can download the build images from the repo [profanOS-build](https://github.com/esolangs/profanOS-build)
or the [latest release](https://github.com/elydre/profanOS/releases/tag/latest)

```bash
# Run the iso image in qemu
qemu-system-i386 -cdrom profanOS.iso

# With KVM acceleration
qemu-system-i386 -cdrom profanOS.iso -enable-kvm
```

profanOS can also be tested online with two clicks with the [v86 copy](https://github.com/copy/v86)
emulator [here](https://elydre.github.io/profan).

For information about real hardware boot and instalation see the [dedicated section](#real-hardware)

### Hardware requirements

profanOS is a 32-bit operating system, it is therefore necessary to have a 32-bit
processor to run it.
There is no exact RAM value to have, but without disk the entire file system is
loaded into memory, however a few megabytes are enough.

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| CPU (x86) | 1@100Mhz| 1@2Ghz      |
| RAM       | 2MB     | 16MB        |
| screen    | text    | 1024x768    |

## First look

When starting profanOS, you will be greeted by the Olivine shell, a language similar to bash.
You can then run the `help` command to see a list of useful commands.

![banner](https://elydre.github.io/img/profan/banner.png)

## The kernel

The profanOS kernel (generally called generic kernel or profan kernel) is at the heart
of the OS, it is extremely minimalist and can be completed by adding libraries
loaded from disk such as drivers or file system extensions.

profanOS is **not** a SASOS - single address space operating system, but part of the memory is shared,
like the kernel and the libraries. Processes can therefore freely access kernel functions.

Here is a list of the main kernel features:

- multiboot support
- 32 bits protected mode
- PS/2 mouse and keyboard
- ATA hard disk
- custom filesystem
- preemptive multi-tasking
- memory allocation
- virtual memory management
- librarys and modules
- ring0 only

## The userspace

### Programing languages

The kernel and userspace are developed mainly in C. The Olivine Shell (see the [language documentation](https://elydre.github.io/md/olivine)) is the main shell language.
You can also use the lua, sulfur, C and C++ languages to create your own programs.

### Major ports

- [lua](https://github.com/elydre/lua-profan) programming language, with custom library
- [doom](https://github.com/elydre/doom-profan) engine, playable with keyboard
- [sulfur](https://github.com/asqel/sulfur_lang) programming language, a project of a friend
- [tcc](https://github.com/elydre/tinycc-profan) compiler, a small and fast c compiler
- [vlink](https://github.com/elydre/vlink-profan) linker with multi-format support

All the ports are available with the command `make addons` or by building them manually.

## Real-Hardware

profanOS works on pc with legacy bios but not with uefi. However profanOS
can work on recent pc by activating bios compatibility.

To install profanOS on a USB key or an internal disk, it is possible to use
the installation script `tools/install.sh` or any other image flasher.

> **Warning** -
> installing an OS on a real machine can be risky and
> must be done with knowledge of the possible risks

### Install on USB key

- Download ISO or build it in linux
- Flash the ISO on the USB key with `dd` or any other image flasher
- Activate the legacy bios in the bios settings (if not already done)
- Boot on the USB key from the bios boot menu
- select the graphical mode

### Install on internal disk

This method is dangerous and will cause the complete erasure of your machine's disk.
Please make a backup of your data and be sure of what you are doing.

- Activate the legacy bios in the bios settings (if not already done)
- Boot on a live linux
- Download ISO or build it in linux
- Flash the ISO with `tools/install.sh` or any other image flasher

```sh
# replace sdX with the disk to flash
sudo sh install.sh /dev/sdX profanOS.iso
```

- Reboot on the internal disk

## About

### Known major bugs

| bug name   | since | description                            | cause | fixed ?   |
|------------|-------|----------------------------------------|-------|-----------|
| lagged lag | ?     | all profanOS is getting very slow      | ?     | partially |
| BOBCAT     | 0.4.2 | some C compiler build broken zlibs     | dily  | no        |
| no KB      | ?     | keyboard not working sometimes         | ?     | no        |

### Screenshots

| ![shell](https://elydre.github.io/img/profan/screen/shell.png) | ![doom](https://elydre.github.io/img/profan/screen/doom.png) |
|------------------------------------------------------------------|-------------------------------------------------------------------|
| ![windaube](https://elydre.github.io/img/profan/screen/windaube.png) | ![lua](https://elydre.github.io/img/profan/screen/lua.png) |

### Author & Contact

- pf4 ([@elydre](https://github.com/elydre))

Contact me on my discord [server](https://discord.gg/PFbymQ3d97) or in PM `@pf4`

### Source & Acknowledgment

- **[os tutorial](https://github.com/cfenollosa/os-tutorial) for the original tutorial**
- **[@Lorisredstone](https://github.com/Lorisredstone) for all the help and ideas**
- [@copy](https://github.com/copy/v86) for the v86 online emulator and floppy build
- [osdev wiki](https://wiki.osdev.org/) for the documentation made by the community
- [framindmap](https://framindmap.org) for the mindmap of the todo list
- [Terry Davis](https://templeos.org) for the inspiration and his courage
- [@yuukidesu9](https://gitlab.com/yuukidesu9/yuuos) for the iso creation
- [@iProgramInCpp](https://github.com/iProgramMC) for vbe pitch help and the inspiring OS *NanoShellOS*
- [szhou42](https://github.com/szhou42/osdev) for the ata driver
- [@ProtoByter](https://github.com/ProtoByter) for the first pull request
- [@asqel](https://github.com/asqel) for the real hardware test and sulfur lang

*be careful with our friend 55*
