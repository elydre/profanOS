# profanOS

this repo originally comes from part 23 of the excellent tutorial [How to create an OS from scratch](https://github.com/cfenollosa/os-tutorial) thanks!

![screenshot](https://github.com/elydre/elydre.github.io/blob/main/img/profanOS.png)

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

# Qemu command
qemu-system-i386 -fda profan-img.bin
```
