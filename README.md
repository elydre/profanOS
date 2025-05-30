# The profan Operating System

[![publish](https://github.com/elydre/profanOS/actions/workflows/publish.yml/badge.svg)](https://github.com/elydre/profanOS/actions/workflows/publish.yml)
[![commit-activity](https://img.shields.io/github/commit-activity/m/elydre/profanOS)](https://github.com/esolangs/profanOS-build/tree/main/img)
[![loc](https://img.shields.io/badge/dynamic/json?color=blue&label=code%20lines&query=profan_lines&url=https://elydre.github.io/build/profan_data.json)](https://github.com/elydre/profanOS/graphs/code-frequency)
[![latest](https://img.shields.io/badge/dynamic/json?color=blue&label=latest%20kernel&query=kernel_version&url=https://elydre.github.io/build/profan_data.json)](https://elydre.github.io/profan)

![wave](https://elydre.github.io/img/profan.svg)

The profan Operating System is an independent OS developed from scratch.
It is characterized by its ring0-only preemptive modular multitasking
minimalist kernel and colorful-looking command line-based user interface.
[Test it from your browser](https://elydre.github.io/profan/latest.html)
without installing anything.

You can find a progress roadmap in [github projet](https://github.com/users/elydre/projects/7),
a [monthly updates](https://github.com/elydre/profanOS/discussions/categories/retrospective) and
the [wiki](https://github.com/elydre/profanOS/wiki) for documentation.

## Setup

> [!NOTE]
> Compilation is guaranteed only on linux with **gcc 11**
> it is also possible in windows with virtualization
> solutions like wsl (on windows 11) or hyperV.

### Install dependencies

```bash
# Debian based
sudo apt update
sudo apt install -y make python3 gcc g++ nasm qemu-system-i386 \
                    grub-common xorriso grub-pc-bin mtools

# Arch based
sudo pacman -Syu
sudo pacman -S      make python gcc nasm qemu-full xorriso \
                    grub-common mtools
```

### Compile & Run

```bash
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

For information about real hardware boot and instalation see the [dedicated section](#real-hardware).

## First look

When starting profanOS, you will be greeted by the Olivine shell, a language similar to bash.
You can then run the `help` command to see a list of useful commands.

![banner](https://elydre.github.io/img/profan/banner.png)

> [!TIP]
> To switch the keyboard layout use the `kb <layout>` command, such as `kb qwerty`.

## The kernel

The profanOS kernel (generally called generic kernel or profan kernel) is at the heart
of the OS, it is extremely minimalist and can be completed by adding modules loaded
from disk such as drivers or file system extensions.

profanOS is **not** a SASOS - single address space operating system, but part of the memory is shared,
like kernel and modules. Processes can therefore freely access kernel memory.

Here is a list of the main kernel features:

- multiboot support
- 32 bits protected mode
- PS/2 mouse and keyboard
- ATA hard disk
- custom filesystem
- preemptive multitasking
- memory allocation
- virtual memory management
- kernel modules
- full ring0

## The userspace

### Programing languages

The kernel and userspace are developed mainly in C. The Olivine Shell (see the
[language documentation](https://elydre.github.io/md/olivine)) is the main shell language.
You can also use python3, posix-sh, lua, perl, sulfur, and C languages to create your
own programs.

If you prefer a bash like rather than Olivine, you can use the `dash` port which is
POSIX compliant and often used as `/bin/sh` in linux systems.

### Major ports - Addons

- [tcc](https://bellard.org/tcc/) Small and fast C compiler
- [python](https://www.python.org/) Python 3.11 interpreter
- [dash](https://github.com/elydre/dash-profan) POSIX compliant shell
- [GNU make](https://www.gnu.org/software/make/) Makefile interpreter
- [lua](https://www.lua.org/) Lightweight scripting language
- [doom](https://github.com/ozkl/doomgeneric) Raycasting first person shooter

All the ports are available with the command `make addons`.
A lot of programs are compiled from [this repo](https://github.com/elydre/libatron),
you can also find a list of stuff tested in profanOS
[here](https://github.com/stars/elydre/lists/compile-in-profan).

### Libraries

Libraries are loaded from file system and are dynamically linked to executables using
`deluge` (profan dynamic linker). Kernel modules, for their part, are shared between
the process and their content are accessible using syscalls.

Here is a list of the main libraries and kernel modules:

- **kernel modules**:
  - devio,
    [filesys](https://github.com/elydre/profanOS/wiki/lib_filesys),
    fmopen,
    [libmmq](https://github.com/elydre/profanOS/wiki/lib_mmq),
    [panda](https://github.com/elydre/profanOS/wiki/lib_panda),
    [profan](https://github.com/elydre/profanOS/wiki/lib_profan)
- **libc** - standard C library
- **libpm** - profanOS minimalistic math lib
- **libpf** - extra stuff like [libtsi](https://github.com/elydre/profanOS/wiki/lib_tsi)

Find all the libraries available on the [wiki](https://github.com/elydre/profanOS/wiki/Dev-Links).

## Real-Hardware

profanOS works on pc with legacy bios but not with uefi. However profanOS
can work on recent pc by activating bios compatibility.

To install profanOS on a USB key or an internal disk, it is possible to use
the installation script `tools/install.sh` or any other image flasher.

> [!WARNING]
> Installing an OS on a real machine can be risky and
> must be done with knowledge of the possible risks!

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

| bug name   | since | description                       | cause           | fixed ? |
|------------|-------|-----------------------------------|-----------------|---------|
| lagged lag | ?     | all profanOS is getting very slow | memory access ? | partial |
| no KB      | ?     | keyboard not working sometimes    | ?               | no      |

### Screenshots

| ![halfix](https://elydre.github.io/img/profan/screen/halfix.png) | ![lua](https://elydre.github.io/img/profan/screen/lua.png) |
|------------------------------------------------------------------|-------------------------------------------------------------------|
| ![windaube](https://elydre.github.io/img/profan/screen/windaube.png) | ![doom](https://elydre.github.io/img/profan/screen/doom.png) |

### Author & Contact

- pf4 ([@elydre](https://github.com/elydre))

Contact me on my discord [server](https://discord.gg/PFbymQ3d97) or in PM `@pf4`

### Source & Acknowledgment

- **[os tutorial](https://github.com/cfenollosa/os-tutorial) for the original tutorial**
- [@asqel](https://github.com/asqel) for tests, the sulfur language and all the ports
- [@Sarah](https://github.com/Sarenard) for all the help and ideas
- [@copy](https://github.com/copy/v86) for the v86 online emulator and floppy build
- [@spaskalev](https://github.com/spaskalev/buddy_alloc) for the buddy allocator
- [osdev wiki](https://wiki.osdev.org/) for documentation made by the community
- [ToaruOS](https://github.com/klange/toaruos) for the inspiration and the dynamic linking
- [mintsuki](https://github.com/mintsuki/freestanding-headers) for freestanding headers

A huge thank you to all the [contributors](https://github.com/elydre/profanOS/graphs/contributors)
and all the people who took the time to look at this project!

*be careful with our friend 55*
